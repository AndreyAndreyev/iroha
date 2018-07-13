// properties([
//   parameters([
//     booleanParam(defaultValue: true, description: 'Build `iroha`', name: 'iroha'),
//     booleanParam(defaultValue: false, description: 'Build `bindings`', name: 'bindings'),
//     booleanParam(defaultValue: true, description: '', name: 'x86_64_linux'),
//     booleanParam(defaultValue: false, description: '', name: 'armv7_linux'),
//     booleanParam(defaultValue: false, description: '', name: 'armv8_linux'),
//     booleanParam(defaultValue: false, description: '', name: 'x86_64_macos'),
//     booleanParam(defaultValue: false, description: '', name: 'x86_64_win'),
//     choice(choices: 'Debug\nRelease', description: 'Iroha build type', name: 'build_type'),
//     booleanParam(defaultValue: false, description: 'Build Java bindings', name: 'JavaBindings'),
//     choice(choices: 'Release\nDebug', description: 'Java bindings build type', name: 'JBBuildType'),
//     string(defaultValue: 'jp.co.soramitsu.iroha', description: 'Java bindings package name', name: 'JBPackageName'),
//     booleanParam(defaultValue: false, description: 'Build Python bindings', name: 'PythonBindings'),
//     choice(choices: 'Release\nDebug', description: 'Python bindings build type', name: 'PBBuildType'),
//     choice(choices: 'python3\npython2', description: 'Python bindings version', name: 'PBVersion'),
//     booleanParam(defaultValue: false, description: 'Build Android bindings', name: 'AndroidBindings'),
//     choice(choices: '26\n25\n24\n23\n22\n21\n20\n19\n18\n17\n16\n15\n14', description: 'Android Bindings ABI Version', name: 'ABABIVersion'),
//     choice(choices: 'Release\nDebug', description: 'Android bindings build type', name: 'ABBuildType'),
//     choice(choices: 'arm64-v8a\narmeabi-v7a\narmeabi\nx86_64\nx86', description: 'Android bindings platform', name: 'ABPlatform'),
//     booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
//     string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')
//   ]),
//   buildDiscarder(logRotator(numToKeepStr: '20')),
//   timestamps()
// ])

def environmentList = []
def environment = [:]
def tasks = [:]

node('master') {
  checkout scm
  environment = [
    "CCACHE_DIR": "/opt/.ccache",
    "DOCKER_REGISTRY_BASENAME": "hyperledger/iroha",
    "IROHA_NETWORK": "iroha-0${env.CHANGE_ID}-${env.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_HOST": "pg-0${env.CHANGE_ID}-${env.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_USER": "pguser${env.GIT_COMMIT}",
    "IROHA_POSTGRES_PASSWORD": "${env.GIT_COMMIT}",
    "IROHA_POSTGRES_PORT": "5432",
    //"WS_DIR=/var/jenkins/workspace/09ea0b41fe86d884c6ecf57676d34ecacfb5411d-30"
    "WS_BASE_DIR": "/var/jenkins/workspace"
  ]
}
environment.each { it ->
  environmentList.add("${it.key}=${it.value}")
}
def agentLabels = ['x86_64-agent': 'x86_64_aws_cross', 'armv8-agent': 'armv8-cross']
def targetOS = ['ubuntu-xenial', 'ubuntu-bionic', 'debian-stretch', 'macos']
def targetArch = ['x86_64': agentLabels['x86_64-agent'], 'arm64': agentLabels['armv8-agent']]

def buildSteps(label, arch, os, buildType, environment) {
  return {
    node(label) {
      withEnv(environment) {
        // checkout to expose env vars
        checkout scm
        sh("mkdir -p ${env.WS_BASE_DIR}/${env.GIT_COMMIT}-${env.BUILD_NUMBER}-${arch}-${os}")
        dir(ws) {
          // then checkout into actual workspace
          checkout scm
          debugBuild = load ".jenkinsci/debug-build-cross.groovy"
          debugBuild.doDebugBuild()
        }
      }
    }
  }
}

def testSteps(label, arch, os, environment) {
  return {
    node(label) {
      withEnv(environment) {
        checkout scm
        dir("${env.WS_BASE_DIR}/${env.GIT_COMMIT}-${env.BUILD_NUMBER}-${arch}-${os}") {
          testBuild = load ".jenkinsci/debug-test.groovy"
          testBuild.doDebugTest()
        }
      }
    }
  }
}

for(int i=0; i < targetOS.size(); i++) {
  targetArch.each { arch ->
    tasks["${targetOS[i]}-${arch.key}"] = {
      return {
        buildSteps(agentLabels['x86_64-agent'], arch.key, targetOS[i], "Debug", environmentList)
        testSteps(arch.value, arch.key, targetOS[i], environmentList)
      }
    }
  }
}

stage('Debug build') {
  parallel tasks
}
