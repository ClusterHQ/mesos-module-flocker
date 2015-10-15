# mesos-module-flocker

Mesos isolator for Flocker volumes.

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

### Env variables in isolator
While resource attributes are being worked on (work in progress at Mesosphere), Flocker Isolator will use environment variables (similar to [dvdi](https://github.com/emccode/mesos-module-dvdi/blob/341d4126ad6de0cec0788d65cb214ae3875748a2/isolator/isolator/docker_volume_driver_isolator.hpp#L207)).

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
