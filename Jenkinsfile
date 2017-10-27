#!/usr/bin/env groovy

def ircNotification(result) {
	def server = "icann.irc.meshnet.pl"
	def port = "6667"
	def nick = "JENC_${BUILD_NUMBER}"
	def channel = '#antinet-build'
	result = result.toUpperCase()
	def message = "Project ${JOB_NAME} build #${BUILD_NUMBER}: ${result}: ${BUILD_URL}"
	node {
		sh """
			(
			echo NICK ${nick}
			echo USER ${nick} 8 * : ${nick}
			sleep 5
			echo "JOIN ${channel}"
			echo "NOTICE ${channel} :${message}"
			echo QUIT
			) | nc ${server} ${port}
		"""
	}
}

def native_linux(git_url, branch) {
	build job: 'galaxy42_linux',
		parameters: [	[$class: 'NodeParameterValue',
		             		name: 'Multinode',
		             		labels: ['debian'],
		             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def native_windows_mingw32(git_url, branch) {
	build job: 'galaxy42_mingw_32bit',
		parameters: [	[$class: 'NodeParameterValue',
		             		name: 'Multinode',
		             		labels: ['win32&&cygwin'],
		             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def native_windows_mingw64(git_url, branch) {
	build job: 'galaxy42_mingw_64bit',
		parameters: [	[$class: 'NodeParameterValue',
		             		name: 'Multinode',
		             		labels: ['win64&&cygwin'],
		             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def native_windows_msvc(git_url, branch) {
	build job: 'galaxy42_MSVC',
		parameters: [	[$class: 'NodeParameterValue',
		             	name: 'Multinode',
		             	labels: ['win64&&msvc'],
		             	nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def run_unit_test(git_url, branch) {
	build job: 'galaxy42_unit-tests',
		parameters: [	[$class: 'LabelParameterValue',
		             		name: 'UnitTest',
		             		label: 'allow_unittests' ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def run_integration_test() {
	build job: 'galaxy42_integration-tests',
		parameters: [	[$class: 'LabelParameterValue',
		             		name: 'Integration',
		             		labels: ['allow_integrationtests'] ] ]
}

def run_memory_test(git_url, branch) {
	build job: 'g42_safe-memory',
		parameters: [	[$class: 'NodeParameterValue',
		             		name: 'MemTest',
		             		labels: ['allow_memtest'],
		             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

def run_thread_ub_test(git_url, branch) {
	build job: 'g42_safe-thread-ub',
		parameters: [	[$class: 'NodeParameterValue',
		             		name: 'ThubTest',
		             		labels: ['allow_thread_ub_test'],
		             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_repository_url',
		             		value: "$git_url" ],
		             	[$class: 'StringParameterValue',
		             		name: 'git_branch',
		             		value: "$branch" ] ]
}

// build trigger - checking changes every 5 min
properties([pipelineTriggers([  [$class: 'GitHubPushTrigger'],
                                [$class: "SCMTrigger", scmpoll_spec: "H/5 * * * *"]
                             ])
          ])

node('master') {
	properties([disableConcurrentBuilds()])

	def build_native_linux = true
	def build_native_windows_mingw32 = true
	def build_native_windows_mingw64 = true
	def build_native_windows_msvc = true

	def should_run_unit_test = true
	def should_run_integration_test = true
	def should_run_memory_test = true
	def should_run_thread_ub_test = true

	def build_gitian_linux = false
	def build_gitian_macosx = false
	def build_gitian_windows = false

	def GIT_REPOSITORY_URL = scm.getUserRemoteConfigs()[0].getUrl()
	println "GIT_URL: [$GIT_REPOSITORY_URL]"

	//git repository branch could be passed on jenkins build configuration as regular expresion, more info: Jenkins Git plugin
	//def GIT_BRANCH = sh(returnStdout: true, script: 'git rev-parse --abbrev-ref HEAD').trim()
	def GIT_BRANCH = ""

	if (env.BRANCH_NAME.startsWith("PR-")) {
		println "Detect Pull Request ${env.BRANCH_NAME}"
		def PRNumber = env.BRANCH_NAME.tokenize("PR-")[0]
		//def gitURLcommand = 'git config --local remote.origin.url'
		//gitURL = sh(returnStdout: true, script: gitURLcommand).trim()
		println "PRNumber=PR number: [$PRNumber]"
		GIT_BRANCH = "origin/pr/${PRNumber}/head"

	} else {
		GIT_BRANCH = "${env.BRANCH_NAME}"
	}
	println "BRANCH: [$GIT_BRANCH]"

	def failure_counter=0

	parallel (
		linux: {
			if(build_native_linux) {
				stage('linux') {
					native_linux(GIT_REPOSITORY_URL,GIT_BRANCH)
				}
			}
		},
		windows_mingw32: {
			if(build_native_windows_mingw32) {
				stage('windows_mingw32') {
					native_windows_mingw32(GIT_REPOSITORY_URL,GIT_BRANCH)
				}
			}
		},
		windows_mingw64: {
			if(build_native_windows_mingw64) {
				stage('windows_mingw64') {
					native_windows_mingw64(GIT_REPOSITORY_URL,GIT_BRANCH)
				}
			}
		},
		windows_msvc: {
			if(build_native_windows_msvc) {
				stage('windows_MSVC') {
					native_windows_msvc(GIT_REPOSITORY_URL,GIT_BRANCH)
				}
			}
		}
	)

	if (should_run_unit_test) {
		stage('unit_test') {
			try {
				run_unit_test(GIT_REPOSITORY_URL,GIT_BRANCH)
			} catch (all) {
				println "Unit_Test fail, {$all}\nbut we continue."
				failure_counter++
			}
		}
	}


	if (should_run_integration_test) {
		stage('integration_tests') {
			try {
				run_integration_test()
			} catch (all) {
				println "Integration_tests probably fails, {$all}\nbut we continue to next stage."
				println "Check individual item build console log for details."
				failure_counter++
			}
		}
	}


	parallel (
		memory_test: {
			if (should_run_memory_test) {
				stage('memory_test') {
					try {
						run_memory_test(GIT_REPOSITORY_URL,GIT_BRANCH)
					} catch (all) {
						println "Memory test fail, {$all}\nbut we continue."
						failure_counter++
					}
				}
			}
		},
		thread_ub_test: {
			if (should_run_thread_ub_test) {
				stage('thread_ub_test') {
					try {
						run_thread_ub_test(GIT_REPOSITORY_URL,GIT_BRANCH)
					} catch (all) {
						println "Thread and undefined behaviors test fail, {$all}\nbut we continue."
						failure_counter++
					}
				}
			}
		}
	)

	if (build_gitian_linux) {
		stage('deterministic_linux') {
			try {
				build job: 'galaxy42_gitian_-L',
					parameters: [	[$class: 'NodeParameterValue',
					             		name: 'Gitian_linux',
					             		labels: ['allow_gitian'],
					             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_repository_url',
					             		value: "${GIT_REPOSITORY_URL}" ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_branch',
					             		value: "${GIT_BRANCH}" ] ]

			} catch (all) {
				println "Gitian_build_linux probably fails, {$all}\nbut we continue to next stage."
				println "Check individual item build console log for details."
				failure_counter++
			}
		}
	}
	if (build_gitian_macosx) {
		stage('deterministic_macosx') {
			try {
				build job: 'galaxy42_gitian_-M',
					parameters: [	[$class: 'NodeParameterValue',
					             		name: 'Gitian_osx',
					             		labels: ['allow_gitian'],
					             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_repository_url',
					             		value: "${GIT_REPOSITORY_URL}" ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_branch',
					             		value: "${GIT_BRANCH}" ] ]
			} catch (all) {
				println "Gitian_build_macosx probably fails, {$all}\nbut we continue to next stage."
				println "Check individual item build console log for details."
				failure_counter++
			}
		}
	}
	if (build_gitian_windows) {
		stage('deterministic_windows') {
			try {
				build job: 'galaxy42_gitian_-W',
					parameters: [	[$class: 'NodeParameterValue',
					             		name: 'Gitian_windows',
					             		labels: ['allow_gitian'],
					             		nodeEligibility: [$class: 'AllNodeEligibility'] ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_repository_url',
					             		value: "${GIT_REPOSITORY_URL}" ],
					             	[$class: 'StringParameterValue',
					             		name: 'git_branch',
					             		value: "${GIT_BRANCH}" ] ]
			} catch (all) {
				println "Gitian_build_windows probably fails, {$all}\nbut we continue to next stage."
				failure_counter++
			}
		}
	}

	if(failure_counter > 0) {
		println "At least one stage has failed, make pipeline build failure"
		currentBuild.result = 'FAILURE'
		println "Check individual item build - console log for details."
	} else {
		currentBuild.result = 'SUCCESS'
	}

	node('debian') {
		ircNotification(currentBuild.result)
	}
}

