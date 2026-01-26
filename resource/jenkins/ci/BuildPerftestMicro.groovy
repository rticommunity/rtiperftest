/* (c) 2026 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

@Library('RTIConnextCommon@feature/PERF-928') _

String extractConnextVersion(String artifactoryPath) {
    if (!artifactoryPath) {
        return null
    }

    List<String> tokens = artifactoryPath.tokenize('/')
    return tokens.size() > 1 ? tokens[1] : null
}

Boolean checkArchitectureIsPSL(String architecture) {
    return architecture.contains('-') ? true : false
}

String getPILarchitecture(String pslArchitecture) {
    if (checkArchitectureIsPSL(pslArchitecture)) {
        return pslArchitecture.tokenize('-')[0]
    }
    return pslArchitecture
}

String getBuildIdentifier(String artifactoryPath) {
    if (!artifactoryPath) {
        return null
    }

    List<String> tokens = artifactoryPath.tokenize('/')
    return tokens.size() > 2 ? tokens[2] : null
}

void setupEnvironment() {
    MICRO_4_VERSION = extractConnextVersion(params.MICRO_4_STAGE)
    MICRO_CERT_VERSION = extractConnextVersion(params.CERT_2_STAGE)
    MICRO_2_VERSION = extractConnextVersion(params.MICRO_2_STAGE)

    BUILD_MATRIX = [
        [
            'version': MICRO_4_VERSION,
            'typeFlag': '--micro',
            'stage': params.MICRO_4_STAGE,
            'architectures': params.MICRO_4_ARCHITECTURES.tokenize(','),
            'datasizes': params.MICRO_4_DATASIZES.tokenize(','),
            'datasizes_extended': params.MICRO_4_DATASIZES_EXTENDED.tokenize(',')
        ],
        [
            'version': MICRO_CERT_VERSION,
            'typeFlag': '--cert',
            'stage': params.CERT_2_STAGE,
            'architectures': params.CERT_ARCHITECTURES.tokenize(','),
            'datasizes': params.CERT_DATASIZES.tokenize(','),
            'datasizes_extended': params.CERT_DATASIZES_EXTENDED.tokenize(',')
        ],
        [
            'version': MICRO_2_VERSION,
            'typeFlag': '--micro-24x-compatibility',
            'stage': params.MICRO_2_STAGE,
            'architectures': params.MICRO_2_ARCHITECTURES.tokenize(','),
            'datasizes': params.MICRO_2_DATASIZES.tokenize(','),
            'datasizes_extended': params.MICRO_2_DATASIZES_EXTENDED.tokenize(',')
        ]
    ]

    BRANCHNAME = (env.CHANGE_ID ? env.CHANGE_BRANCH : env.BRANCH_NAME)
}

void downloadArtifact(String artifactPath) {
    echo "Downloading files from ${artifactPath}"
    rtDownload(
        serverId: getRtiArtifactoryServerId(),
        spec:
        """{
            "files": [
                {
                    "pattern": "${artifactPath}",
                    "explode": "true",
                    "flat": "true"
                }
            ]
        }"""
    )
}

void downloadBundles(entry, architecture) {
    downloadArtifact("${entry.stage}/staging/host_src_core.tgz")
    if (checkArchitectureIsPSL(architecture)) {
        String pilArchitecture = getPILarchitecture(architecture)
        downloadArtifact("${entry.stage}/staging/${pilArchitecture}.tgz")
        downloadArtifact("${entry.stage}/staging/host_src_psl.tgz")
    }
    downloadArtifact("${entry.stage}/staging/host_resource.tgz")
    downloadArtifact("${entry.stage}/staging/${architecture}.tgz")
    downloadArtifact("${entry.stage}/internalBuild/rti_me*.tgz")
}

void stashBundles(entry, architecture, zeroCopy) {
    String folder = "bin/${architecture}/release"
    String bundleSuffix = zeroCopy ? '-zc' : ''
    String bundleExtension = architecture.contains('Win') ? 'zip' : 'tar.gz'
    String bundleName = "rti-perftest${bundleSuffix}"
    String fileName = "${bundleName}.${bundleExtension}"
    String command = architecture.contains('Win') ? "cd ${folder} && zip -r ../../../" : "tar -C ${folder} -czvf "

    runCommand "${command}${fileName} ."

    stash name: "${bundleName}-${entry.version}-${architecture}", includes: fileName
}

List<Map<String, Closure>> generateBuildStages(Boolean zeroCopy) {
    Map<String, Closure> stages = [:]
    BUILD_MATRIX.each { entry ->
        entry.architectures.each { arch ->
            stages["${entry.version}-${arch}"] = {
                stage("${entry.version}-${arch}") {
                    def nodeData = [:]
                    nodeData = nodeMgr.determineAndPrepareNode(
                        arch,
                        '',
                        'ssh://git@bitbucket.rti.com:7999/perf/rti-perftest.git',
                        BRANCHNAME,
                        'resource/jenkins/docker'
                    )

                    if (nodeMgr.nodeIsDocker(nodeMgr.getNode(nodeData))) {
                        node(nodeMgr.getNode(nodeData)) {
                            stage('Build') {
                                deleteDir()
                            }
                        }
                    }
                    nodeMgr.runInBuildNode(null, nodeData) {
                        checkout scm
                        println "Building perftest for ${entry.version} on ${arch}"
                        buildPerftestMicro(entry, arch, zeroCopy, arch.contains('CERT'))
                    }
                }
            }
        }
    }
    return stages
}

List<Map<String, Closure>> generateUploadBundleStage(){
    Map<String, Closure> stages = [:]
    stages['Uploads'] = {
        stage('Uploads') {
            node('docker') {
                docker.image('repo.rti.com:443/build-docker-prod-hq/docbuilder:latest')
                .inside() {
                    for (entry in BUILD_MATRIX) {
                        for (arch in entry.architectures) {
                            uploadBundle(entry, arch)
                        }
                    }
                }
            }
        }
    }
    return stages
}

void buildPerftestMicro(entry, architecture, zeroCopy, isCert) {
    downloadBundles(entry, architecture)

    String sourceDir  = isCert ? 'rti_me_cert.1.0' : 'rti_me.2.0'
    String targetDir  = "rti_connext_dds_${isCert ? 'cert' : 'micro'}-${entry.version}"

    sh "cp -r ${sourceDir}/rtiddsgen ${targetDir}"
    sh "cp -r ${sourceDir}/resource ${targetDir}"
    sh "cp -r ${sourceDir}/include ${targetDir}"

    sh "chmod +x ${targetDir}/rtiddsgen/scripts/rtiddsgen"
    if (isCert) {
        sh "chmod +x ${targetDir}/resource/scripts/rtime-make"
    }

    List<String> zeroCopyFlags = ['--no-zeroCopy']
    if (zeroCopy) {
        zeroCopyFlags = []
        List<String> datasizes = entry.datasizes + entry.datasizes_extended
        for (datasize in datasizes) {
            zeroCopyFlags += "--cert-zc-datalen ${datasize}"
        }
    }

    for (zeroCopyFlag in zeroCopyFlags) {
        sh "./build.sh --cpp-build ${entry.typeFlag} --rtimehome ${env.WORKSPACE}/${targetDir} --platform ${architecture} ${zeroCopyFlag}"
    }

    stashBundles(entry, architecture, zeroCopy)
}

void uploadBundle(entry, architecture) {
    String bundleExtension = architecture.contains('Win') ? 'zip' : 'tar.gz'
    String bundleName = 'rti-perftest'
    String bundleNameZC = 'rti-perftest-zc'

    unstash "${bundleName}-${entry.version}-${architecture}"
    unstash "${bundleNameZC}-${entry.version}-${architecture}"

    rtUpload (
        serverId: getRtiArtifactoryServerId(),
        spec:
        """{
            "files": [
                {
                    "pattern": "${bundleName}*.${bundleExtension}",
                    "target": "perftest-ci/${BRANCHNAME}/connextmicro/${entry.version}/${getBuildIdentifier(entry.stage)}/${architecture}/",
                    "props": "architecture=${architecture}"
                }
            ]
        }"""
    )
}

pipeline {
    agent none

    parameters {
        string(name: 'MICRO_4_STAGE', defaultValue: 'connextmicro/4.2.0/BUILD_4.2.0_FINAL_RTI_REL')
        string(name: 'MICRO_4_ARCHITECTURES', defaultValue: 'x86_64leElfgcc7.3.0-Linux4')
        string(name: 'MICRO_4_DATASIZES', defaultValue: '32,64,128,256,512,1024,2048,4096,8192,16384,32768,63000')
        string(name: 'MICRO_4_DATASIZES_EXTENDED', defaultValue: '100000,500000,1048576,1548576,4194304,10485760')

        string(name: 'CERT_2_STAGE', defaultValue: 'connextmicro/2.4.16/BUILD_2.4.16_20250922T184540Z_RTI_REL')
        string(name: 'CERT_ARCHITECTURES', defaultValue: 'x86_64leElfgcc7.3.0CERT-Linux4')
        string(name: 'CERT_DATASIZES', defaultValue: '32,64,128,256,512,1024,2048,4096,8192,16384,32768,63000')
        string(name: 'CERT_DATASIZES_EXTENDED', defaultValue: '100000,500000,1048576,1548576,4194304,10485760')

        string(name: 'MICRO_2_STAGE', defaultValue: 'connextmicro/2.4.14.3/BUILD_2.4.14.3_20251011T174848Z_RTI_REL')
        string(name: 'MICRO_2_ARCHITECTURES', defaultValue: 'x64Linux4gcc7.3.0')
        string(name: 'MICRO_2_DATASIZES', defaultValue: '32,64,128,256,512,1024,2048,4096,8192,16384,32768,63000')
        string(name: 'MICRO_2_DATASIZES_EXTENDED', defaultValue: '100000,500000,1048576,1548576,4194304,10485760')

        booleanParam(
            name: 'UPLOAD_BUNDLES_TO_ARTIFACTORY',
            defaultValue: false,
            description: 'Confirm if you want to upload the bundles to Artifactory.'
        )
    }

    stages {
        stage('Set-Up Environment') {
            steps {
                script {
                    setupEnvironment()
                }
            }
        }
        stage('Build Perftest') {
            steps {
                script {
                    Boolean zeroCopy = false
                    parallel generateBuildStages(zeroCopy)
                }
            }
        }
        stage('Build Perftest ZC') {
            steps {
                script {
                    Boolean zeroCopy = true
                    parallel generateBuildStages(zeroCopy)
                }
            }
        }
        stage('Upload Bundles to Artifactory') {
            when {
                expression { return params.UPLOAD_BUNDLES_TO_ARTIFACTORY }
            }
            steps {
                script {
                    parallel generateUploadBundleStage()
                }
            }
        }
    }
}
