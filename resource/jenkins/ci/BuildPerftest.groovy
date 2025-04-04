/* (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

def buildPerftestAgainstPro(String connextArch, String nddsHome, String flags) {
    sh "./build.sh --platform ${connextArch} --nddshome ${nddsHome} ${flags}"
}

pipeline {
    agent none
    parameters {
        string(name: 'DEVELOP_VERSION', defaultValue: 'latest', description: 'RTI Connext Pro version to build against')
        string(name: 'LATEST_RELEASE_VERSION', defaultValue: '7.4.0', description: 'RTI Connext Pro version to build against')
        string(name: 'LATEST_LTS_VERSION', defaultValue: '7.3.0', description: 'RTI Connext Pro version to build against')
    }
    stages {
        stage('Build Perftest against Connext in develop') {
            matrix {
                axes {
                    axis {
                        name 'CONNEXT_ARCH'
                        values 'x64Linux4gcc7.3.0', 'armv8Linux4gcc7.3.0'
                    }
                    axis {
                        name 'LANGUAGE_FLAG'
                        values '--cpp-build', '--cpp11-build', '--java-build', '--cs-build'
                    }
                }
                agent {
                    dockerfile {
                        filename 'resource/jenkins/docker/Dockerfile'
                        additionalBuildArgs "--build-arg PLATFORM=${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? 'amd64' : 'arm64'} --build-arg TAG=${params.DEVELOP_VERSION}"
                        args "${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? '--platform linux/amd64' : '--platform linux/arm64'}"
                    }
                }
                stages {
                    stage('Build') {
                        steps {
                            echo "Building perftest against RTI Connext Pro (tag: ${params.DEVELOP_VERSION}) ${CONNEXT_ARCH} with ${LANGUAGE_FLAG}"
                            buildPerftestAgainstPro(CONNEXT_ARCH, '$NDDSHOME', "${LANGUAGE_FLAG} --secure")
                        }
                    }
                }
            }
        }
        stage('Build Perftest against Latest Release') {
            matrix {
                axes {
                    axis {
                        name 'CONNEXT_ARCH'
                        values 'x64Linux4gcc7.3.0', 'armv8Linux4gcc7.3.0'
                    }
                    axis {
                        name 'LANGUAGE_FLAG'
                        values '--cpp-build', '--cpp11-build', '--java-build', '--cs-build'
                    }
                }
                agent {
                    dockerfile {
                        filename 'resource/jenkins/docker/Dockerfile'
                        additionalBuildArgs "--build-arg PLATFORM=${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? 'amd64' : 'arm64'} --build-arg TAG=${params.LATEST_RELEASE_VERSION}"
                        args "${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? '--platform linux/amd64' : '--platform linux/arm64'}"
                    }
                }
                stages {
                    stage('Build') {
                        steps {
                            echo "Building perftest against RTI Connext Pro (tag: ${params.LATEST_RELEASE_VERSION}) ${CONNEXT_ARCH} with ${LANGUAGE_FLAG}"
                            buildPerftestAgainstPro(CONNEXT_ARCH, '$NDDSHOME', "${LANGUAGE_FLAG} --secure")
                        }
                    }
                }
            }
        }
        stage('Build Perftest against Latest LTS') {
            matrix {
                axes {
                    axis {
                        name 'CONNEXT_ARCH'
                        values 'x64Linux4gcc7.3.0', 'armv8Linux4gcc7.3.0'
                    }
                    axis {
                        name 'LANGUAGE_FLAG'
                        values '--cpp-build', '--cpp11-build', '--java-build', '--cs-build'
                    }
                }
                agent {
                    dockerfile {
                        filename 'resource/jenkins/docker/Dockerfile'
                        additionalBuildArgs "--build-arg PLATFORM=${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? 'amd64' : 'arm64'} --build-arg TAG=${params.LATEST_LTS_VERSION}"
                        args "${CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? '--platform linux/amd64' : '--platform linux/arm64'}"
                    }
                }
                stages {
                    stage('Build') {
                        steps {
                            echo "Building perftest against RTI Connext Pro (tag: ${params.LATEST_LTS_VERSION}) ${CONNEXT_ARCH} with ${LANGUAGE_FLAG}"
                            buildPerftestAgainstPro(CONNEXT_ARCH, '$NDDSHOME', "${LANGUAGE_FLAG} --secure")
                        }
                    }
                }
            }
        }
    }
}
