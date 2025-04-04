/* groovylint-disable NestedBlockDepth */
/* (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

String UPLOAD_PATH = "doozer@sjc01kvm2:~/www-docs/docs/rti-perftest/${params.DOC_SERVER_DESTINATION_PATH}"

pipeline {
    agent none
    parameters{
        string(
            name: 'DOC_SERVER_DESTINATION_PATH',
            defaultValue: "${env.BRANCH_NAME}",
            description: 'Target path to upload the documentation.'
        )
        booleanParam(
            name: 'UPLOAD_DOCUMENTATION_TO_DOC_SERVER',
            defaultValue: false,
            description: 'Confirm if you want to upload the documentation to the documentation server.'
        )
    }
    options {
        disableConcurrentBuilds()
        /*
            To avoid excessive resource usage in server, we limit the number
            of builds to keep in pull requests
        */
        buildDiscarder(
            logRotator(
                artifactDaysToKeepStr: '',
                artifactNumToKeepStr: '',
                daysToKeepStr: '',
                /*
                   For pull requests only keep the last 20 builds, for regular
                   branches keep up to 100 builds.
                */
                numToKeepStr: env.CHANGE_URL ? '20' : '100'
            )
        )
        // Set a timeout for the entire pipeline
        timeout(time: 10, unit: 'HOURS')
    }
    stages {
        stage('Build documentation') {
            agent {
                docker {
                    label "docker"
                    image 'repo.rti.com:443/build-docker-prod-hq/docbuilder:latest'
                    alwaysPull true
                }
            }
            steps {
                script {
                    currentBuild.description="""<a href="${env.BUILD_URL}/RTI_20Perftest_20Documentation/">RTI Perftest Documentation</a>"""
                    echo "Building documentation..."
                    echo "Result will be available at ${env.BUILD_URL}/RTI_20Perftest_20Documentation/ and https://docserver.rti.com/docs/rti-perftest/${params.DOC_SERVER_DESTINATION_PATH}"
                    runCommand './build.sh --build-doc'
                }
            }
            post {
                success {
                    /* Publish the documentation preview in Jenkins */
                    publishHTML(
                        [
                            allowMissing: false,
                            alwaysLinkToLastBuild: true,
                            keepAll: true,
                            reportDir: 'doc/html/',
                            reportFiles: 'index.html',
                            reportName: 'RTI Perftest Documentation',
                            reportTitles: 'RTI Perftest Documentation'
                        ]
                    )
                    stash includes: 'doc/', name: 'perftest-doc'
                }
                failure {
                    error('Error building the documentation')
                }
            }
        }
        stage('Upload documentation') {
            agent {
                docker {
                    label "docker"
                    image 'repo.rti.com:443/build-docker-prod-hq/docbuilder:latest'
                    alwaysPull true
                }
            }
            when {
                expression {
                    params.UPLOAD_DOCUMENTATION_TO_DOC_SERVER == true
                }
            }
            steps {
                unstash 'perftest-doc'
                sshagent(credentials: ['doozer-microservice-deploy-sshkey']) {
                    sh("""ssh -o StrictHostKeyChecking=no doozer@sjc01kvm2 'mkdir -p ~/www-docs/docs/rti-perftest/${params.DOC_SERVER_DESTINATION_PATH}'""")
                    sh("rsync -arv --delete -e 'ssh -o StrictHostKeyChecking=no' doc/html/ ${UPLOAD_PATH}")
                    sh("rsync -arv --delete -e 'ssh -o StrictHostKeyChecking=no' doc/pdf/ ${UPLOAD_PATH}/pdf")
                }
            }
            post {
                success {
                    slackSend(
                        message: ":white_check_mark: Perftest documentation is ready! You can check it <${env.BUILD_URL}/RTI_20Perftest_20Documentation/|here> and in the documentation server <https://docserver.rti.com/docs/rti-perftest/${params.DOC_SERVER_DESTINATION_PATH}/|here>",
                        channel: slackUserIdFromEmail(currentBuild.getBuildCauses()[0].userId+"@rti.com"),
                        botUser: true
                    )
                }
                failure {
                    slackSend(
                        message: ":atlerror: Your build has failed. An error ocurred during the documentation build. Check the details <${env.BUILD_URL}|here>",
                        channel: slackUserIdFromEmail(currentBuild.getBuildCauses()[0].userId+"@rti.com"),
                        botUser: true
                    )
                    error("Error ocurred uploading the documentation to ${UPLOAD_PATH}")
                }
            }
        }
    }
}
