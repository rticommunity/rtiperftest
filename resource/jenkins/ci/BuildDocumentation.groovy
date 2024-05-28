/* groovylint-disable NestedBlockDepth */
/* (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

pipeline {
    agent none
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
                    echo "Result will be available at ${env.BUILD_URL}/RTI_20Perftest_20Documentation/"
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
                }
                failure {
                    error('Error building the documentation')
                }
            }
        }
    }
}
