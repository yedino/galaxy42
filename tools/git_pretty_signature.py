import sys
import os
import argparse
from subprocess import check_output, call, check_call, Popen, PIPE, CalledProcessError, STDOUT
import locale

# the colors info
class BColors:

	def __init__(self):
		pass

	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'

	def disable(self):  # remove all colors
		self.HEADER = ''
		self.OKBLUE = ''
		self.OKGREEN = ''
		self.WARNING = ''
		self.FAIL = ''
		self.ENDC = ''


# count signatures types (e.g. good/bad/...) if check_mode then will exit at first problem,
# encoding current system encoding
# pager - is the pipe to some pager e.g. to the "less" process
def totag_stats(pager, encoding, check_mode=False):
	mainloop_count = 0
	ch = 't'  # should we quit now? (maybe not needed now with the ls?)
	# count
	G_totag = 0  # good
	U_totag = 0  # untrust
	B_totag = 0  # bad
	N_totag = 0  # none (not signed - or maybe can not check e.g. can not find pubkey)
	while ch != 'q':

		# the command to get log:
		data_com = 'git log -1 --skip='+str(mainloop_count)+' --pretty=format:"%G?%d"'

		try:
			data = check_output(data_com.split()).decode(encoding)
		except CalledProcessError as err:
			pager.stdin.write('exit(1)'.encode(encoding))
			exit(1)

		# the status of git signature check - parsed from git log
		# the letters G,U,B,N are here because of the git pretty format used above
		sig_stat = data[1:2]
		if sig_stat == 'G':
			G_totag += 1
		elif sig_stat == 'U':
			U_totag += 1
		elif sig_stat == 'B':
			B_totag += 1
		elif sig_stat == 'N':
			N_totag += 1

		col_end = BColors.ENDC
		col_tag = BColors.OKGREEN

		# find the "tag" in the log - to see the lines with information about the git tag
		# TODO secure this?
		tag_pos = data.find("tag")

		if tag_pos != -1 or not data:  # if there is a tag - or if we should end our work because no data

			# getting tag from %d ref names
			if tag_pos != -1:
				# if it's really tag?
				hash_com = 'git log -1 --skip='+str(mainloop_count)+' --pretty=format:"%H"'
				try:
					commit_hash = check_output(hash_com.split()).decode(encoding)
					# pager.stdin.write(commit_hash.encode(encoding))  # dbg
				except CalledProcessError:
					pass
				if not commit_point_to_tag(commit_hash[1: -1]):
					mainloop_count += 1
					continue

				tag_info = data[tag_pos:-1]
				tag_end_pos = tag_info.find(',')
				tag_info = tag_info[:tag_end_pos]

				# last tag checking (the newest tag - to which we are counting)
				if sig_stat == 'G':
					G_totag -= 1
				if sig_stat == 'U':
					U_totag -= 1
					col_tag = BColors.WARNING
					string = col_tag+"WARNING: Last tag have untrusted sign!"+col_end+'\n'
					pager.stdin.write(string.encode(encoding))
					if check_mode:
						exit(1)
				if sig_stat == 'B':
					B_totag -= 1
					col_tag = BColors.FAIL
					string = col_tag+"ERROR: Last tag have bad sign!!"+col_end+'\n'
					pager.stdin.write(string.encode(encoding))
					if check_mode:
						exit(1)
				if sig_stat == 'N':
					N_totag -= 1
					col_tag = BColors.FAIL
					string = col_tag+"ERROR: Last tag is not signed!!"+col_end+'\n'
					pager.stdin.write(string.encode(encoding))
					if check_mode:
						exit(1)

			col_coms = BColors.OKGREEN  # default colour to use if all ok

			# commits checking
			# if any problems:
			if U_totag:
				col_coms = BColors.OKBLUE
				string = col_coms+"NOTICE: At least one commit with untrusted sign!"+col_end+'\n'
				pager.stdin.write(string.encode(encoding))
				if check_mode:
					exit(1)
			if B_totag:
				col_coms = BColors.FAIL
				string = col_coms+"ERROR: At least one commit with bad sign!"+col_end+'\n'
				pager.stdin.write(string.encode(encoding))
				if check_mode:
					exit(1)
			if N_totag:
				col_coms = BColors.WARNING
				string = col_coms+"WARNING: At leas one not signed commit!!"+col_end+'\n'
				pager.stdin.write(string.encode(encoding))
				if check_mode:
					exit(1)

			# if there was some tag:
			if tag_pos != -1:
				info_head = col_coms+"Signatures status after "+col_tag+tag_info+":\n"
				if col_tag == BColors.OKGREEN and (col_coms == BColors.OKGREEN or col_coms == BColors.OKBLUE):
					info_ft = col_coms+"OK: Everything SINCE LAST TAG "+tag_info[5:]+" seems correctly signed\n"
				else:
					info_ft = col_coms+"WARNING: There is at least one problem with signatures SINCE LAST TAG "+tag_info[5:]+'\n'
			else:
				info_head = col_coms+"Signatures status for all commits:\n"
				info_ft = BColors.OKBLUE + "NOTICE: No tag found in this repository\n"

			# info_head = "\n\n\n" + "Warning: (bug#m145) check if no branches, remotes (refnames) contain word 'tag' otherwise "\
			#			+ "this parsing can be not accurate and could show not correct stats. Also read the log yourself "\
			#			+ " to make sure.\n\n" + info_head

			info_str = info_head                                    \
						+ col_coms + str(G_totag) + ": GOOD\n"      \
						+ col_coms+str(U_totag) + ": UNTRUSTED\n"   \
						+ col_coms+str(B_totag) + ": BAD SIGN\n"    \
						+ col_coms+str(N_totag) + ": NOT SIGNED (or can not check signature)\n"  \
						+ col_coms+info_ft+col_end + '\n'
			pager.stdin.write(info_str.encode(encoding))
			return
		mainloop_count += 1


# include unannotated tags !
def is_tag(tag_name):
	try:
		check_call(['git', 'describe', '--tags', tag_name], stderr=STDOUT, stdout=open(os.devnull, 'w'))
		return True
	except CalledProcessError:
		return False


# return True if commit point to annotated tag
# False otherwise
def commit_point_to_tag(commit_hash):
	try:
		check_call(['git', 'describe', '--exact-match', commit_hash], stderr=STDOUT, stdout=open(os.devnull, 'w'))
		return True
	except CalledProcessError:
		return False


# quick test is git working here, is this git working directory:
def is_git_directory(path='.'):
	return call(['git', '-C', path, 'branch'], stderr=STDOUT, stdout=open(os.devnull, 'w')) == 0

if __name__ == "__main__":

	if not is_git_directory():
		print("Exit: This directory does not contain a git repository!")
		exit(1)

	try:
		encoding = locale.getdefaultlocale()[1]
		pager = Popen(['less', '-F', '-R', '-S', '-X', '-K'], stdin=PIPE, stdout=sys.stdout)

		parser = argparse.ArgumentParser(description='Display git log with gpg signatures in pretty colored format')
		parser.add_argument('-c', '--check',action="store_true",
							help='running check mode, exit with 1 status if occurs warning or error')

		args = parser.parse_args()
		if args.check:
			totag_stats(pager, encoding, True)
		else:
			totag_stats(pager, encoding)

		i = 0 # the position (number of commits - top skip since newest one, since start of git log now)
		# main loop:
		while True:
			data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%H:%G?%d"'

			try:
				data = check_output(data_com.split()).decode(encoding)
			except CalledProcessError as e:
				pager.stdin.write('exit(1)'.encode(encoding))
				exit(1)

			if not data:
				break     # end of commits success exit

			del_pos = data.find(':')
			com_hash = data[1:del_pos]
			sig_stat = data[del_pos+1:del_pos+2]

			com = "git||log||-1||"+com_hash+"||--pretty=format:\"%C(yellow)commit: %H%C(auto)%d\n"
			sig_info = ""
			ref_info = ""
			sig_info_com = "git log "+com_hash+" -1 --pretty=format:\"%GG\""
			ref_info_com = "git log "+com_hash+" -1 --pretty=format:\"%d\""

			if sys.version_info > (3, 0):
				sig_info = check_output(sig_info_com.split()).decode(encoding)
				ref_info = check_output(ref_info_com.split()).decode(encoding)
			else:
				sig_info = check_output(sig_info_com.split())
				ref_info = check_output(ref_info_com.split())

			tag_pos = ref_info.find("tag")
			# getting tag from %d ref names
			tag_table = ""
			if tag_pos != -1 and commit_point_to_tag(com_hash):  # show a tag info:

				tag_info = ref_info[tag_pos:-1]
				tag_end_pos = tag_info.find(',')
				tag_info = tag_info[:tag_end_pos]
				info_len = len(com_hash) + len(ref_info) - 2
				info_len_cor = info_len - len(tag_info) - 13

				tag_table = "%C(yellow reverse)" + '='*info_len + '\n'
				tag_table += "%C(yellow reverse)===%C(reset)%C(yellow) " + tag_info + " is here "
				tag_table += "%C(yellow reverse)" + '='*info_len_cor + '\n'

				tag = tag_info[5:]		# 5 - for skip "tag: "
				tag_verbouse_com = "git tag -v " + tag

				# git throw exception when tag have
				# no detailed info
				try:
					tag_verbouse = check_output(tag_verbouse_com.split(), stderr=STDOUT, universal_newlines=True)
							#
							#									shell=True,
							#					timeout=3,

				except CalledProcessError as err:
					tag_verbouse = "short tag - no detailed information\n"
					# tag_verbouse += err.output    # output from stderr

				tag_table += "%C(yellow reverse)==%C(reset)%C(yellow) info:\n"
				tag_table += "%C(yellow reverse)=%C(reset)%C(yellow) "
				tag_table += tag_verbouse.replace('\n', '\n%C(yellow reverse)=%C(reset)%C(yellow) ')

				tag_table += "\b%C(yellow reverse)" + '='*(info_len-1) + '\n'

			sig_info = repr(sig_info).replace('"', '\\"')[2:-2]
			sig_info = sig_info.replace('\\n', '\n')
			sig_info = sig_info.replace('\\\"', '\"')
			if sig_stat == 'G':
				tag_table = tag_table.replace('gpg', '%C(green)SIGNATURE - GOOD\n%C(reverse yellow)=%C(reset) gpg', 1)
				com += tag_table.replace('gpg:', '%C(green)gpg:')
				com += "%C(green)SIGNATURE - GOOD\n%C(green)"
				com += sig_info.replace('\n', '\n%C(green)')[1:-1]
			elif sig_stat == 'U':
				tag_table = tag_table.replace('gpg', '%C(cyan)SIGNATURE - UNTRUSTED\n%C(reverse yellow)=%C(reset) gpg', 1)
				com += tag_table.replace('gpg:', '%C(cyan)gpg:')
				com += "%C(cyan)SIGNATURE - UNTRUSTED\n%C(cyan)"
				com += sig_info.replace('\n', '\n%C(cyan)')[1:-1]
			elif sig_stat == 'B':
				tag_table = tag_table.replace('gpg', '%C(magneta)SIGNATURE - BAD\n%C(reverse yellow)=%C(reset) gpg', 1)
				com += tag_table.replace('gpg:', '%C(magneta)gpg:')
				com += "%C(magneta reverse)SIGNATURE - BAD SIGN\n%C(magneta reverse)"
				com += sig_info.replace('\n', '\n%C(magneta)')[1:-1]
			elif sig_stat == 'N':
				tag_table = tag_table.replace('gpg', '%C(red)SIGNATURE - FAIL\n%C(reverse yellow)=%C(reset) gpg', 1)
				com += tag_table.replace('gpg:', '%C(red)gpg:')
				com += '%C(red reverse)! !!! ! WARNING ! !!! !\n' 		\
					+ '%C(red)! SIGNATURE - FAILURE !\n' 				\
					+ '%C(red reverse)! !!! ! WARNING ! !!! !\n' 		\
					+ '%C(red)'

				sig_info = sig_info.replace('\n', '\n%C(red)')[1:-1]
				sig_info = sig_info.replace('\\\'', '\'')
				com += sig_info

			com += "%C(white)Author: %aN <%aE>\n%C(white)Date: %aD\n\n    %B\n\""

			try:
				if sys.version_info > (3, 0):
					data = check_output(com.split("||")).decode(encoding)
					data = data[1:-1]
				else:
					data = check_output(com.split("||"))
					data = data[1:-1]

				pager.stdin.write(data.encode(encoding))

			except CalledProcessError as e:
				pager.stdin.write('exit(1)'.encode(encoding))
				exit(1)

			i += 1

		pager.stdin.close()
		pager.wait()

	except IOError as e:
		if e.errno == 32:  # catch q quit from less
			pass
		else:
			print("Unexpected error: " + os.strerror(e.errno))
			raise

	except KeyboardInterrupt:
		# less should handle this
		pass
