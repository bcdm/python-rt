#!/usr/bin/env python

from numpy.distutils.core import setup, Extension
from os import environ as env

version = '0.1'

define_macros=[
   ('PY_RT_VERSION',          '\\\"%s\\\"' % version),
   ]

extra_compile_args=['-Wall']

setup(name='python-rt',
      version=version,
      description='Real time extensions for python',
      long_description='Real time extensions for python.',
      author='Matthieu Bec',
      author_email='mdcb808@gmail.com',
      url='https://github.com/mdcb/python-rt',
      license='GNU General Public License',
      ext_modules=[
         Extension(
            name='rt',
            sources = [
               'python-rt.c',
              ],
            define_macros=define_macros,
            libraries = ['rt'],
            extra_compile_args = extra_compile_args,
            )
         ]
      )
     
