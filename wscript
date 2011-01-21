#!/usr/env/python
# coding=utf8

# You must set the path to Boost and Qt in the waf options; i.e.:
#
# python waf/waf configure --boost-includes=/path/to/boost --qtdir=/path/to/qt --qtbin=/path/to/qt/bin --qtlibs=/path/to/qt/lib
#

import os


def options(opt):
    opt.load('compiler_cxx boost qt4')


def configure(conf):
    conf.load('compiler_cxx boost qt4')
    conf.check_boost()


def build(bld):
    bld.program(
        source = os.path.join('demo', 'main.cpp'),
        target = 'sigfwd_demo',
        features = 'qt4 cxx',
        uselib = 'QTCORE QTGUI',
        includes = ['include'],
        cxxflags = ['-O2', '-Wall']
    )

# use the following if you want to add the include paths automatically
from waflib.TaskGen import feature, before, after
@feature('cxx')
@after('process_source')
@before('apply_incpaths')
def add_includes_paths(self):
    incs = set(self.to_list(getattr(self, 'includes', '')))
    for x in self.compiled_tasks:
        incs.add(x.inputs[0].parent.path_from(self.path))
    self.includes = list(incs)
