from distutils.core import setup, Extension

LargeVis = Extension('LargeVis',
                    sources = ['LargeVis.cpp', 'LargeVismodule.cpp', 'ANNOY\\mman.cpp'],
                    depends=['LargeVis.h'],
                    include_dirs = ['D:\\boost_1_58_0'],
                    library_dirs = ['D:\\boost_1_58_0\\stage\\lib'],
                    extra_compile_args=['/Ox'])

setup (name = 'LargeVis',
       version = '1.0',
       description = 'LargeVis',
       ext_modules = [LargeVis])
	   
