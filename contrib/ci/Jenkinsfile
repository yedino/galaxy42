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

node('master') {
	def GIT_REPOSITORY_URL="https://github.com/yedino/galaxy42.git"

	// git repository branch could be passed as regular expresion, more info: Jenkins Git plugin
	def GIT_BRANCH="*/dev"

	def failure_counter=0

	stage('native_build_parallel') {
		parallel linux: {
	//stage 'linux_build'
			build job: 'galaxy42_native',
				parameters: [	[$class: 'NodeParameterValue',
									name: 'Multinode',
									labels: ['debian'],
									nodeEligibility: [$class: 'AllNodeEligibility'] ],
								[$class: 'StringParameterValue',
									name: 'git_repository_url',
									value: "${GIT_REPOSITORY_URL}" ],
								[$class: 'StringParameterValue',
									name: 'git_branch',
									value: "${GIT_BRANCH}" ] ]
		},
		windows: {
	//stage 'windows_build'
			build job: 'galaxy42_native',
				parameters: [	[$class: 'NodeParameterValue',
									name: 'Multinode',
									labels: ['win32&&cygwin'],
									nodeEligibility: [$class: 'AllNodeEligibility'] ],
								[$class: 'StringParameterValue',
									name: 'git_repository_url',
									value: "${GIT_REPOSITORY_URL}" ],
								[$class: 'StringParameterValue',
									name: 'git_branch',
									value: "${GIT_BRANCH}" ] ]
		}
	}

	stage('unit_test') {
		try {
			build job: 'galaxy42_unit-tests',
				parameters: [	[$class: 'LabelParameterValue',
									name: 'Unit',
									label: 'allow_unittests' ],
								[$class: 'StringParameterValue',
									name: 'git_repository_url',
									value: "${GIT_REPOSITORY_URL}" ],
								[$class: 'StringParameterValue',
									name: 'git_branch',
									value: "${GIT_BRANCH}" ] ]
		} catch (all) {
				println "Integration_tests probably fails, but we continue to next stage."
				println "Check individual item build console log for details."
				failure_counter++
		}
	}

	stage('integration_tests') {
		try {
			build job: 'galaxy42_integration-tests',
				parameters: [	[$class: 'LabelParameterValue',
									name: 'Integration',
									label: 'allow_integrationtests' ] ]

		} catch (all) {
			println "Integration_tests probably fails, but we continue to next stage."
			println "Check individual item build console log for details."
			failure_counter++
		}
	}

	stage('Gitian_build_linux') {
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
			println "Gitian_build_linux probably fails, but we continue to next stage."
			println "Check individual item build console log for details."
			failure_counter++
		}
	}
	stage('Gitian_build_macosx') {
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
			println "Gitian_build_macosx probably fails, but we continue to next stage."
			println "Check individual item build console log for details."
			failure_counter++
		}
	}
	stage('Gitian_build_windows') {
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
			println "Gitian_build_windows probably fails, but we continue to next stage."
			failure_counter++
		}
	}

	if(failure_counter > 0) {
		println "At least one stage has failed, make pipeline build failure"
		currentBuild.result = 'FAILURE'
		println "Check individual item build - console log for details."
	}

	ircNotification(currentBuild.result)
}
