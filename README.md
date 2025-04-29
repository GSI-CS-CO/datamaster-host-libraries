# Datamaster Host Build
This is the project meant to build the datamaster host stack

# How to build

# Project Structure
The project contains the modules that are supposed to be built, currently beeing:
* Datamaster-Host-Libraries: The carpeDM library
* fesa-class-generator: The "Generator" FESA-Class
* fesa-du-generator: The "Generator" Deploy-Unit

# Technical Dept
* Boost is not used from the provided SDK but from a manual build
    * See https://git.acc.gsi.de/timing/datamaster-yocto
    * To get this working, adjustments had to be done in scripts/_build.sh
    * The next SDK may include a compatible boost version which will make this step obsolete
* Missing Config scripts for projects
