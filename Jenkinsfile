properties([
  parameters([
    booleanParam(defaultValue: true, description: 'Build `iroha`', name: 'iroha'),
    booleanParam(defaultValue: false, description: 'Build `bindings`', name: 'bindings'),
    booleanParam(defaultValue: false, name: 'amd64'),
    booleanParam(defaultValue: true, name: 'arm64'),
    booleanParam(defaultValue: false, name: 'armhf'),
    booleanParam(defaultValue: false, name: 'ubuntu-xenial'),
    booleanParam(defaultValue: false, name: 'ubuntu-bionic'),
    booleanParam(defaultValue: true, name: 'debian-stretch'),
    booleanParam(defaultValue: false, name: 'macos'),
    booleanParam(defaultValue: false, name: 'windows'),
    choice(choices: 'Debug\nRelease', description: 'Iroha build type', name: 'IrohaBuildType'),
    booleanParam(defaultValue: false, description: 'Build Java bindings', name: 'JavaBindings'),
    choice(choices: 'Release\nDebug', description: 'Java bindings build type', name: 'JBBuildType'),
    string(defaultValue: 'jp.co.soramitsu.iroha', description: 'Java bindings package name', name: 'JBPackageName'),
    booleanParam(defaultValue: false, description: 'Build Python bindings', name: 'PythonBindings'),
    choice(choices: 'Release\nDebug', description: 'Python bindings build type', name: 'PBBuildType'),
    choice(choices: 'python3\npython2', description: 'Python bindings version', name: 'PBVersion'),
    booleanParam(defaultValue: false, description: 'Build Android bindings', name: 'AndroidBindings'),
    choice(choices: '26\n25\n24\n23\n22\n21\n20\n19\n18\n17\n16\n15\n14', description: 'Android Bindings ABI Version', name: 'ABABIVersion'),
    choice(choices: 'Release\nDebug', description: 'Android bindings build type', name: 'ABBuildType'),
    choice(choices: 'arm64-v8a\narmeabi-v7a\narmeabi\nx86_64\nx86', description: 'Android bindings platform', name: 'ABPlatform'),
    booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
    string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')
  ]),
  buildDiscarder(logRotator(numToKeepStr: '20'))
])

def environmentList = []
def environment = [:]
def builders =
  [
  'build':[
    (agentLabels['amd64-agent']):
    [
      new Tuple('amd64', 'ubuntu-xenial'),
      new Tuple('amd64', 'ubuntu-bionic'),
      new Tuple('amd64', 'debian-stretch'),
      new Tuple('arm64', 'ubuntu-xenial'),
      new Tuple('arm64', 'ubuntu-bionic'),
      new Tuple('arm64', 'debian-stretch'),
      new Tuple('armhf', 'ubuntu-xenial'),
      new Tuple('armhf', 'ubuntu-bionic'),
      new Tuple('armhf', 'debian-stretch')],
    (agentLabels['mac-agent']):
    [
      new Tuple('amd64', 'macos')],
    (agentLabels['windows-agent']):
    [
      new Tuple('amd64', 'windows')
    ]],
  'test':[
    (agentLabels['amd64-agent']):
    [
      new Tuple('amd64', 'ubuntu-xenial'),
      new Tuple('amd64', 'ubuntu-bionic'),
      new Tuple('amd64', 'debian-stretch')],
    (agentLabels['arm64-agent']):
    [
      new Tuple('arm64', 'ubuntu-xenial'),
      new Tuple('arm64', 'ubuntu-bionic'),
      new Tuple('arm64', 'debian-stretch')],
    (agentLabels['arm64-agent']):
    [
      new Tuple('armhf', 'ubuntu-xenial'),
      new Tuple('armhf', 'ubuntu-bionic'),
      new Tuple('armhf', 'debian-stretch')],
    (agentLabels['mac-agent']):
    [
      new Tuple('amd64', 'macos')],
    (agentLabels['windows-agent']):
    [
      new Tuple('amd64', 'windows')]
    ]]


def userInputArchOsTuples() {
  combinationsTuples = []
  m = ['arch': ['amd64': amd64, 'arm64': arm64, 'armhf': armhf],
       'os'  : ['ubuntu-xenial': ubuntu-xenial,
                'ubuntu-bionic': ubuntu-bionic,
                'debian-stretch': debian-stretch,
                'macos': macos,
                'windows': windows]]
  mArch = m['arch'].findAll { it.value == true }.collect { it.key }
  mOs = m['os'].findAll { it.value == true }.collect { it.key }
  combinationsList = GroovyCollections.combinations([mArch, mOs])
  combinationsList.each { it ->
    combinationsTuples.add(new Tuple(it[0], it[1]))
  }
  return combinationsTuples
}

node('master') {
  def scmVars = checkout scm
  environment = [
    "CCACHE_DIR": "/opt/.ccache",
    "DOCKER_REGISTRY_BASENAME": "hyperledger/iroha",
    "IROHA_NETWORK": "iroha-0${scmVars.CHANGE_ID}-${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_HOST": "pg-0${scmVars.CHANGE_ID}-${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_USER": "pguser${scmVars.GIT_COMMIT}",
    "IROHA_POSTGRES_PASSWORD": "${scmVars.GIT_COMMIT}",
    "IROHA_POSTGRES_PORT": "5432",
    "WS_BASE_DIR": "/var/jenkins/workspace"
  ]
  sh("echo build type ${params.binaries_build_type}")
}
environment.each { it ->
  environmentList.add("${it.key}=${it.value}")
}

def buildSteps(label, arch, os, buildType, environment) {
  return {
    node(label) {
      withEnv(environment) {
        // checkout to expose env vars
        def scmVars = checkout scm
        //def workspace = "/var/jenkins/workspace/97acaa2bc1fa1db62e6a0531901e0f41886422ce-99-arm64-debian-stretch"
        def workspace = "${env.WS_BASE_DIR}/${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}-${arch}-${os}"
        sh("mkdir -p $workspace")
        dir(workspace) {
          // then checkout into actual workspace
          checkout scm
          debugBuild = load ".jenkinsci/debug-build-cross.groovy"
          debugBuild.doDebugBuild(arch, os, buildType, workspace)
        }
      }
    }
  }
}

def testSteps(label, arch, os, environment) {
  return {
    node(label) {
      withEnv(environment) {
        def scmVars = checkout scm
        def workspace = "${env.WS_BASE_DIR}/${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}-${arch}-${os}"
        //def workspace = "/var/jenkins/workspace/97acaa2bc1fa1db62e6a0531901e0f41886422ce-99-arm64-debian-stretch"
        dir(workspace) {
          testBuild = load ".jenkinsci/debug-test.groovy"
          testBuild.doDebugTest(workspace)
        }
      }
    }
  }
}

def tasks = [:]
// build binaries
if(what_to_build == 'binaries') {
  builders = builders['build'].each { k, v -> v.retainAll(userInputArchOsTuples() as Object[])}
  builders.each { agent, platform ->
    for(int i=0; i < platform.size(); i++) {
      tasks["${agent}-${platform[i][0]}-${platform}[i][1]"] = {
        buildSteps(agent, platform[i][0], platform[i][1], params.IrohaBuildType, environmentList)()
      }
    }
    stage('Build & Test') {
      parallel tasks
    }
  }
}

// build bindings
// build docs
