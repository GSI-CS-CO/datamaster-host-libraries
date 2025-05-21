#!/bin/bash

podman run  --log-driver=none -v .:/workspaces/datamaster-host-build -it ftm-builder /bin/bash
