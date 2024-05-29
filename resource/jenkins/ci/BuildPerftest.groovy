/* (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

def buildPerftestAgainstPro(connext_arch, nddshome, flags) {
    sh "./build.sh --platform ${connext_arch} --nddshome ${nddshome} ${flags}"
}

pipeline {
    agent none

    stages {
        stage('Build Perftest against RTI Connext Pro') {
            matrix {
                axes {
                    axis {
                        name 'CONNEXT_ARCH'
                        values 'x64Linux4gcc7.3.0', 'armv8Linux4gcc7.3.0'
                    }
                    axis {
                        name 'LANGUAGE_FLAG'
                        values '--cpp-build', '--cpp11-build', '--java-build'
                    }
                }
                agent {
                    docker {
                        image "repo.rti.com/connext-containers/connext-sdk:BUILD_7.3.0.0_20240308T000000Z_RTI_REL-20240521"
                        args CONNEXT_ARCH == 'x64Linux4gcc7.3.0' ? '--platform linux/amd64' : '--platform linux/arm64'
                    }
                }
                stages {
                    stage("Build") {
                        steps {
                            buildPerftestAgainstPro("${CONNEXT_ARCH}", "\$NDDSHOME", "${LANGUAGE_FLAG} --secure")
                        }
                    }
                }
            }
        }
    }
}