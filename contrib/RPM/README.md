# freeDiameter RPM Build Guide

This guide outlines the steps required to build the `freeDiameter` project from source on **Rocky Linux 9**, including creating the RPM build structure, enabling necessary repositories, installing required packages, and running the build process.

## Prerequisites

- A **Rocky Linux 9** environment
- **Root or sudo** access to the system
- Basic familiarity with **RPM packaging** and **command-line tools**

## Step 1: Install Required Dependencies

Before building the RPMs, you need to install several dependencies and enable the necessary repositories.

### 1.1 Enable Required Repositories

Some packages, like `json-c-devel`, are in the **CodeReady Builder (CRB)** repository, which is not enabled by default. You also may need the **EPEL** repository for other dependencies.

Run the following commands to enable the repositories:

```bash
# Enable the EPEL repository
sudo dnf install epel-release

# Enable the CRB (CodeReady Builder) repository for extra development packages
sudo dnf config-manager --set-enabled crb

### 1.2 Install Development Packages
Once the repositories are enabled, install the required development libraries:

sudo dnf install json-c-devel libidn-devel libpcap-devel cmake gcc make gnutls-devel libidn-devel libtool pkgconfig sqlite-devel
This will install all the dependencies required for building freeDiameter.

## Step 2: Set Up RPM Build Environment
To build RPMs, you need a specific directory structure and certain build tools.

### 2.1 Create RPM Build Directories
Run the following commands to create the RPM build directories:

# Create the RPM build directory structure
mkdir -p ~/rpmbuild/{SOURCES,SPECS,BUILD,RPMS,SRPMS}

### 2.2 Download the Source Code
Next, download the freeDiameter source code, either from GitHub or a release tarball. For this guide, we will assume you are using the tarball.

cd ~/rpmbuild/SOURCES
# Download the source tarball (replace with the correct URL)
wget https://github.com/freeDiameter/freeDiameter/archive/refs/tags/v1.5.0.tar.gz



# RPM Packaging for freeDiameter

## How to Build

1. Clone the repo and checkout your branch.
2. Create a tarball:
   ```bash
   git archive --format=tar.gz --prefix=freeDiameter-1.5.0/ HEAD > freeDiameter-1.5.0.tar.gz
