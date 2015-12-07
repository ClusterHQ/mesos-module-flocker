# mesos-module-flocker

This is a proof of concept. It is not recommended for production systems.

# Motivation

Enable storage managed by Flocker to be consumed by tasks launched on mesos slave nodes.

## Architecture

![screen shot](https://raw.github.com/clusterhq/mesos-module-flocker/master/img/mesos-flocker-isolator.png "fig 1. overview")


## Design Decisions

### Integration with Flocker Control Service

Flocker Isolator integrates with Flocker Control Service (as opposed to Docker Flocker Plugin, diverging from [DVDI design](https://github.com/emccode/mesos-module-dvdi/blob/master/README.md) ) for the following reasons:

- Reduce overhead of plumbing through Docker Flocker Plugin.
- Use richer feature set exposed by Flocker Control Service.
- Flexibility for usage with containerizers other than Docker.

## Building
We have provided a dockerfile that helps you build the project. Note that the dockerfile uses ubuntu 14.04. We have only tested on an ubuntu 14.04 server. Other linux distributions will differ.

To build and test the project, follow the steps below:
### Mac (assumes your docker-machine is called dev and you have eval'ed for docker)
```
git clone https://github.com/ClusterHQ/mesos-module-flocker.git
cd mesos-module-flocker/docker/mesos-modules-dev
docker build -t containersol/mesos-modules-dev:14.04 .
cd ../..
rm -rf ./build ./bin 
docker-machine ssh dev 'rm -rf /tmp/build/*' 
docker-machine scp -r /Volumes/source/clusterhq/mesos-module-flocker/. dev:/tmp/build 
docker run -it --env MESOS_ROOT=/mesos -v /tmp/build:/build containersol/mesos-modules-dev:14.04 sh -c 'cd /build ; cmake . ; make ; ./build/test_flocker_isolator' 
docker-machine scp -r dev:/tmp/build/bin /Volumes/source/clusterhq/mesos-module-flocker
```
### Linux
```
git clone https://github.com/ClusterHQ/mesos-module-flocker.git
cd mesos-module-flocker/docker/mesos-modules-dev
sudo docker build -t containersol/mesos-modules-dev:14.04 .
cd ../..
rm -rf ./build ./bin 
mkdir build
docker run -it --env MESOS_ROOT=/mesos -v ./build:/build containersol/mesos-modules-dev:14.04 sh -c 'cd /build ; cmake . ; make ; ./build/test_flocker_isolator'
```
## Testing
To run the tests, first build the project then run the application `./build/test_flocker_isolator`. This will run through the gtests in the test folder.

## Installing on Mesos
We have also provided a set of terraform/bash scripts to install a mesos cluster with a working flocker configuration. If you need help creating a cluster, definitely check this out: https://github.com/philwinder/mesos-flocker-terraform

Copy the built `bin/libisolator.so` file to all slaves in your mesos cluster. Next, create a file named `modules.json` which has the settings for your cluster:
```
 {
   "libraries": [
     {
       "file": "/home/ubuntu/libisolator.so",
       "modules": [
         {
           "name": "com_clusterhq_flocker_FlockerIsolator",
           "parameters": [
             {
               "key": "ipaddress",
               "value": "$(YOUR_MASTER_HOSTNAME_OR_IP)"
             },
             {
               "key": "port",
               "value": "4523"
             }
           ]
         }
       ]
     }
   ]
 }
 ```
Now enable the module on the slave with:
```
printf /home/ubuntu/modules.json | sudo tee /etc/mesos-slave/modules
printf com_clusterhq_flocker_FlockerIsolator | sudo tee /etc/mesos-slave/isolation
sudo service mesos-slave restart
```
## Running your stateful application
To use flocker as a backend, you must pass several environmental variables along with your applicaiton. Note that this does not work with the Mesos Docker containerizer, since it is not implemented. See: https://issues.apache.org/jira/browse/MESOS-2840. If you want to run a docker container, run it from the cmd (see example).

```
    "FLOCKER_CONTAINER_VOLUME_PATH": The path that the stateful application will write to
    "FLOCKER_ID": A unique id representing the dataset of this webapp
```
For example:
```
{
  "id":"flocker-webapp-demo",
  "cpus": 0.5,
  "mem": 128,
  "instances": 1,
  "cmd": "sudo docker run --net=bridge -p 8500:80 -v /tmp/data:/data binocarlos/moby-counter:localfile",
  "env": {
    "FLOCKER_CONTAINER_VOLUME_PATH": "/tmp/data",
    "FLOCKER_ID": "webapp-data"
  },
  "ports":[
      8500
    ]
}
```

## Known issues
- You have to delete the symbolic link from your applications path (e.g. /tmp/data in the above example) each time you want to start an application.
- You have to delete datasets manually
- We have only tested with this dockerfile to build and running on the cluster created by https://github.com/philwinder/mesos-flocker-terraform
- This does not work with the docker containerizer, because the docker containerizer does not call any of Mesos's hooks or isolators. See: https://issues.apache.org/jira/browse/MESOS-2840
- If you reuse a FLOCKER_ID of a dataset that has been deleted, it will fail trying to reconnect to a deleted dataset

## Whats Next

### Flocker Resource Provider

Flocker Mesos Isolator allows Flocker volume lifecycle management in a reactive fashion to task start/stop workflows.

In addition to creating/deleting volumes through Mesos Slave, Flocker can also advertize storage capabilities to Mesos Master, to be presented to Frameworks. Storage capabilities include:

Available resources:
- available capacity
- available storage characteristics (compression, dedup, replication factor, storage types (ssd, hdd, ..), etc)
- available storage profiles

Used resources:
- used capacity
- used storage characteristics 
- stats for usage (percentage of storage saved by compression, amount deduped, etc)

Presenting these rich cluster-wide storage data to Mesos Master (and eventually Frameworks) will enable frameworks to make intelligent decisions regarding initial placement of applications.


![screen shot](https://raw.github.com/clusterhq/mesos-module-flocker/master/img/flocker-resource-provider.png "fig 2. overview")

ETA on Resource Provider availability is January 2016.

## Acknowledgements

Many thanks to Adam Bordelon and Michael Park at Mesosphere for productive discussions around the design!

This project was developed by Container Solutions and sponsored by ClusterHQ.
