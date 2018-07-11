#!/usr/bin/env groovy

def doDebugTest() {
  docker.image('postgres:9.5').withRun(""
  + " -e POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
  + " -e POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
  + " --name ${env.IROHA_POSTGRES_HOST}"
  + " --network=${env.IROHA_NETWORK}") {
    docker.image('local/linux-test-env').inside(""
	+ " -e IROHA_POSTGRES_HOST=${env.IROHA_POSTGRES_HOST}"
	+ " -e IROHA_POSTGRES_PORT=${env.IROHA_POSTGRES_PORT}"
	+ " -e IROHA_POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
	+ " -e IROHA_POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
	+ " --network=${env.IROHA_NETWORK}"
	+ " -v ${WS_DIR}:${WS_DIR}") {
	  sh "cp $WS_DIR/build/shared_libs/* /usr/lib/"
	  sh "cd build; ctest --output-on-failure"
	}
  }
}

return this
