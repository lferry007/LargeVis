from distutils.core import setup, Extension
import numpy as np

LargeVis = Extension('LargeVis',
                    sources = ['LargeVis.cpp', 'LargeVismodule.cpp'],
                    depends=['LargeVis.h'],
                    include_dirs = ['/usr/local/include', np.get_include()],
                    library_dirs = ['/usr/local/lib'],
                    libraries=['gsl', 'gslcblas'],
                    extra_compile_args=['-lm -pthread -lgsl -lgslcblas -Ofast -march=native -ffast-math'])

setup (name = 'LargeVis',
       version = '1.0',
       description = 'LargeVis',
       ext_modules = [LargeVis])
	   
