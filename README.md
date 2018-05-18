## Spacelapser

![Example of Spacelapser being used.](docs/images/gui.gif)

Spacelapser is a tool for exploring the three-dimensional volumes created by loading an entire video into memory. Depending on the motion of the camera, this volume can resemble a lightfield, a slit-scan camera, or a special relativity simulator.

![Example of Spacelapser output.](docs/images/vertical_motion.gif)

![Example of Spacelapser output.](docs/images/ocean.gif)

### Installation

The project was built in OpenFrameworks and should be cross-platform. It is tested built with Xcode on OS X.

#### Ubuntu

1. [Install OpenFrameworks:](https://openframeworks.cc/setup/linux-install/)

```
$ git clone https://github.com/openframeworks/openFrameworks.git
$ cd openFrameworks/
$ sudo ./scripts/linux/ubuntu/install_dependencies.sh
$ sudo ./scripts/linux/ubuntu/install_codecs.sh
$ ./scripts/linux/compileOF.sh
```

2. Build Spacelapser:

```
$ git clone https://github.com/loganwilliams/spacelapser.git
$ cd ./spacelapser
$ make
```
