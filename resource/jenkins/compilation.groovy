/*
 * (c) 2023 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or revisions thereof,
 * must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/* import jenkins packet hudson.model for dates */
import java.text.SimpleDateFormat

/* Global variables used in multiple stages of the pipeline */
executionMachine = 'bld-ubuntu1804.rti.com'
user = 'perfuser'
perftestRepo = 'ssh://git@bitbucket.rti.com:7999/perf/rti-perftest.git'
destinationFolderBase = "/home/perfuser/jenkins/perftest-testing"
platform = "x64Linux4gcc7.3.0"

pipeline {
    agent none
    parameters {
        string(
            name: 'NDDSHOME',
            defaultValue: '/home/perfuser/ndds/7.2.0/rti_connext_dds-7.2.0',
            description: 'The Connext DDS Pro Home folder we are going to use to compile against.'
        )
    }
    options {
        skipDefaultCheckout()
        disableConcurrentBuilds()
    }

    stages {
        stage ('Process parameters') {
            agent {
                label 'docker'
            }
            steps {
                script {

                    perftestBRANCH = env.CHANGE_BRANCH ?
                        env.CHANGE_BRANCH :
                        env.BRANCH_NAME

                    if (params.NDDSHOME.trim()) {
                        NDDSHOME= params.NDDSHOME
                    } else {
                        echo "ERROR: NDDSHOME cannot be empty"
                        currentBuild.result = "FAILURE"
                        return 1
                    }

                    def curDate = new Date()
                    def date = new SimpleDateFormat("yyyyMMdd")
                    date.timeZone = java.util.TimeZone.getTimeZone('America/Los_Angeles')
                    date.format(curDate)
                    dateId = date.format(curDate)

                    destinationFolder = "${destinationFolderBase}/${perftestBRANCH}"
                    destinationFolderId = "${destinationFolder}/${dateId}"

                    echo " - NDDSHOME: ${NDDSHOME}\n - Perftest branch: ${perftestBRANCH}\n - Destination Folder: ${destinationFolder}"

                    perftestCompilationCommand = "./build.sh \
                            --platform ${platform} \
                            --nddshome ${NDDSHOME} \
                            --cpp-build \
                            --cpp11-build"
                }
            }
        }
        stage('Compile Perftest against ConnextDDS Pro') {
            agent {
                label "docker"
            }
            steps {
                script {
                    sshagent(credentials: ["$user-ssh-key"]) {
                        sh("""ssh -o StrictHostKeyChecking=no $user@$executionMachine \
                            "rm -rf ${destinationFolder} && \
                            mkdir -p ${destinationFolder} && \
                            /usr/bin/git clone ${perftestRepo} ${destinationFolderId} && \
                            exit 0"
                        """)
                    }
                    sshagent(credentials: ["$user-ssh-key"]) {
                        sh("""ssh -o StrictHostKeyChecking=no $user@$executionMachine \
                            "cd ${destinationFolderId} && \
                            ${perftestCompilationCommand}"
                        """)
                    }
                }
            }
            post {
                cleanup {
                    cleanWs()
                }
            }
        } // end Perftest Compilation
    }
}
