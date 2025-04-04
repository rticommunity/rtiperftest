/* (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
* No duplications, whole or partial, manual or electronic, may be made
* without express written permission.  Any such copies, or revisions thereof,
* must display this notice unaltered.
* This code contains trade secrets of Real-Time Innovations, Inc.
*/

////////////////////ENV///////////////////
CONNEXT_VERSION = ''
CONNEXT_VERSION_SHORT = ''
BUILD_IDENTIFIER = ''
BRANCHNAME = ''
ARCHITECTURES = []
LANGUAGES = []
//////////////////////////////////////////

////////////////////UTILS/////////////////
def extractConnextVersion(String artifactoryPath) {
    def matcher = artifactoryPath =~ /release(\d+)\.(\d+)\.(\d+)(?:\.(\d+))?/
    if (matcher.find()) {
        def major = matcher[0][1]
        def minor = matcher[0][2]
        def patch = matcher[0][3]
        def build = matcher[0][4] // might be null
        return build ? "${major}.${minor}.${patch}.${build}" : "${major}.${minor}.${patch}"
    } else {
        error("Could not extract Connext version from path: ${artifactoryPath}")
    }
}

def extractBuildIdentifier(String artifactoryPath) {
    def matcher = artifactoryPath =~ /(BUILD_\d+\.\d+\.\d+\.\d+_\d{8}T\d{6}Z_RTI_REL)/
    if (matcher.find()) {
        return matcher[0][1]
    } else {
        error("Could not extract build identifier from path: ${artifactoryPath}")
    }
}

def getScriptExtension() {
    return isUnix() ? 'sh' : 'bat'
}

def downloadArtifactoryStage(String connextArch) {
    echo "Downloading files for ${connextArch}"
    def pattern = ARTIFACTORY_PATH + "/staging/connextdds-staging-${connextArch}.tgz"
    rtDownload(
        serverId: getRtiArtifactoryServerId(),
        spec:
        """{
            "files": [
                {
                    "pattern": "${pattern}",
                    "target": "${env.WORKSPACE}/connextdds-staging-${connextArch}.tgz",
                    "flat": "true"
                }
            ]
        }"""
    )
    runCommand "tar -xzf connextdds-staging-${connextArch}.tgz"
}

def getVSDevCmdPatternForArch(String connextArch) {
    def archMap = [
        'x64Win64VS2017': "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\*\\Common7\\Tools\\VsDevCmd.bat",
        'x64Win64VS2015': "C:\\Program Files (x86)\\Microsoft Visual Studio\\2015\\*\\Common7\\Tools\\VsDevCmd.bat"
    ]
    return archMap[connextArch] ?: ""
}

def getScriptExtensionForArch(String connextArch) {
    return connextArch.contains('Win') ? 'zip' : 'tar.gz'
}
/////////////////////////////////////////

///////////Stages Generation////////////
def generateBuildStages(List<String> architectures, List<String> languages) {
    def stages = [:]
    architectures.each { arch ->
        stages[arch] = {
            node(arch) {
                stage(arch) {
                    cleanWs()
                    checkout scm
                    downloadArtifactoryStage(arch)
                    languages.each { lang ->
                        stage(lang) {
                            buildPerftestAgainstPro("${arch}", "${env.WORKSPACE}/unlicensed/rti_connext_dds-${CONNEXT_VERSION_SHORT}", "--${lang} --secure --openssl-home ${env.WORKSPACE}/unlicensed/rti_connext_dds-${CONNEXT_VERSION_SHORT}/third_party/openssl-3.0.12/${arch} --rtiddsgen-path ${env.WORKSPACE}/unlicensed/rti_rtiddsgen/bin/rtiddsgen")                        }
                    }
                }
            }
        }
    }
    return stages
}
def generateUploadBundleStageContent() {
    def stages = [:]
    stages['Results'] = {
        stage('Results') {
            node('docker') {
                docker.image('repo.rti.com:443/build-docker-prod-hq/docbuilder:latest')
                .inside() {
                    try {
                        checkout scm
                        script {
                            runCommand './build.sh --build-doc'
                            for (arch in ARCHITECTURES) {
                                generateBundle(arch)
                            }
                        }
                    }
                    catch (Exception e) {
                            error('Error building the uploading packages: ' + e.message)
                    }
                }
            }
        }
    }
    return stages
}
/////////////////////////////////////////

////////////Stages Functions////////////
def buildPerftestAgainstPro(String connextArch, String nddsHome, String flags) {
    def vsDevCmdPath = ""
    if (!isUnix()) {
        try {
            vsDevCmdPath = powershell(script: """
            \$file = Get-ChildItem '${getVSDevCmdPatternForArch(connextArch)}' -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
            if (-not \$file) { exit 1 }
            Write-Output \$file
        """, returnStdout: true).trim()
        } catch (err) {
            error "VSDevCmd.bat not found for architecture: ${connextArch}"
        }
    }

    withEnv(["VSDEVCMD=${vsDevCmdPath}"]) {
        echo "Building Perftest against Connext Pro ${connextArch}"
        runCommand "./build.${getScriptExtension()} --platform ${connextArch} --nddshome ${nddsHome} ${flags}"
    }
    stash includes: 'bin/', name: "perftest-${connextArch}"
}

def generateBundle(String arch) {
    unstash "perftest-${arch}"
    def folder = "rti_perftest_Connext-${arch}"

    runCommand "mkdir -p ${folder}/bin"
    runCommand "mkdir -p ${folder}/resource"
    runCommand "mkdir -p ${folder}/doc"

    runCommand "cp -r bin/${arch}             ${folder}/bin/"
    runCommand "cp -r resource/secure         ${folder}/resource/"
    runCommand "cp -r doc/*                   ${folder}/doc/"
    runCommand "cp -r perftest_qos_profiles.xml ${folder}/"

    if (!arch.contains('Win')) {
        runCommand """
            tar czvf ${folder}.tar.gz ${folder}
        """
    } else {
        runCommand """
            zip -r ${folder}.zip ${folder}/
        """
    }
    rtUpload (
        serverId: getRtiArtifactoryServerId(),
        spec:
            """{
            "files": [
                {
                    "pattern": "${env.WORKSPACE}/${folder}.${getScriptExtensionForArch(arch)}",
                    "target": "perftest-ci/${BRANCHNAME}/connext/${CONNEXT_VERSION}/${BUILD_IDENTIFIER}/${folder}.${getScriptExtensionForArch(arch)}",
                    "props": "rti.artifact.kind=bundle;rti.product.name=perftest;rti.product.version=${CONNEXT_VERSION}"
                }
            ]
            }""",
        failNoOp: true
    )
}
/////////////////////////////////////////

pipeline {
    agent none

    parameters {
        string(
            name: 'ARTIFACTORY_PATH',
            defaultValue: 'connext-ci/pro/release7.5.0.0/BUILD_7.5.0.0_20250313T000000Z_RTI_REL',
            description: 'Path where connext will be pulled i.e. connext-ci/pro/release7.5.0.0/BUILD_7.5.0.0_20250313T000000Z_RTI_REL/installers',
            trim: true
        )
        string(
            name: 'CONNEXT_BUILD_PLATFORMS',
            defaultValue: 'arm64Darwin20clang12.0,armv8Linux4gcc7.3.0,x64Linux4gcc7.3.0,x64Win64VS2017',
            description: 'Comma-separated list of architectures.',
            trim: true
        )
        string(
            name: 'CONNEXT_BUILD_LANGUAGES',
            defaultValue: 'cpp-build',
            description: 'Comma-separated list of language flags to build.',
            trim: true
        )
    }

    stages {
        stage('Set-Up Environment') {
            steps {
                script {
                    ARCHITECTURES = params.CONNEXT_BUILD_PLATFORMS
                        .split(',')
                        .collect { it.trim() as String }

                    LANGUAGES = params.CONNEXT_BUILD_LANGUAGES
                        .split(',')
                        .collect { it.trim() as String }

                    CONNEXT_VERSION = extractConnextVersion(params.ARTIFACTORY_PATH)
                    CONNEXT_VERSION_SHORT = CONNEXT_VERSION.tokenize('.')[0..2].join('.')
                    BUILD_IDENTIFIER = extractBuildIdentifier(params.ARTIFACTORY_PATH)
                    BRANCHNAME = (env.CHANGE_ID ? env.CHANGE_BRANCH : env.BRANCH_NAME).tokenize('/')[-1]
                    echo "Artifactory Path: ${params.ARTIFACTORY_PATH}"
                    echo "Build Identifier: ${BUILD_IDENTIFIER}"
                    echo "Connext Version: ${CONNEXT_VERSION}"
                    echo "Connext Version Short: ${CONNEXT_VERSION_SHORT}"
                    echo "Perftest Version: ${BRANCHNAME}"
                    echo "Architectures: ${ARCHITECTURES}"
                    echo "Languages: ${LANGUAGES}"
                }
            }
        }
        stage('Build Perftest') {
            steps {
                script {
                    parallel generateBuildStages(ARCHITECTURES, LANGUAGES)
                }
            }
        }
        stage('Upload Bundles') {
            steps {
                script {
                    parallel generateUploadBundleStageContent()
                }
            }
        }
    }
}