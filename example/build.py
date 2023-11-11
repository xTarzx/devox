#!/usr/bin/env python3

import os
from devox import Devox as DVX

dvx = DVX("totri")
dvx.add_src("main.cpp")
dvx.link_lib("raylib")
dvx.build()
