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

## Whats Next

![screen shot](https://raw.github.com/clusterhq/mesos-module-flocker/master/img/flocker-resource-provider.png "fig 2. overview")
