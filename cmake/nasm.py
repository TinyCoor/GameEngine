#!/usr/bin/env python
# -*- coding: utf-8 -*-
from conans import ConanFile, AutoToolsBuildEnvironment, tools

import os
import shutil

class NASMInstallerConan(ConanFile):
    name = "nasm_installer"
    version = "2.13.02"
    url = "https://github.com/bincrafters/conan-nasm_installer"
    description = "The Netwide Assembler, NASM, is an 80x86 and x86-64 assembler designed for portability " \
                  "and modularity"
    license = "http://repo.or.cz/nasm.git/blob/HEAD:/INSTALL"
    exports_sources = ["LICENSE"]
    settings = "os_build", "arch_build", "compiler"
    _source_subfolder = "sources"
    def source(self):
        source_url = "http://www.nasm.us/pub/nasm/releasebuilds/%s/nasm-%s.tar.bz2" % (self.version, self.version)
        tools.get(source_url)
        extracted_dir = "nasm-" + self.version
        os.rename(extracted_dir, self._source_subfolder)
    def _build_vs(self):
        with tools.chdir(self._source_subfolder):
            with tools.vcvars(self.settings, arch=str(self.settings.arch_build), force=True):
                self.run('nmake /f Mkfiles\\msvc.mak')
                # some libraries look for nasmw (e.g. libmp3lame)
                shutil.copy('nasm.exe', 'nasmw.exe')
                shutil.copy('ndisasm.exe', 'ndisasmw.exe')
    def _build_configure(self):
        with tools.chdir(self._source_subfolder):
            cc = os.environ.get('CC', 'gcc')
            cxx = os.environ.get('CXX', 'g++')
            if self.settings.arch_build == 'x86':
                cc += ' -m32'
                cxx += ' -m32'
            elif self.settings.arch_build == 'x86_64':
                cc += ' -m64'
                cxx += ' -m64'
            env_build = AutoToolsBuildEnvironment(self)
            env_build_vars = env_build.vars
            env_build_vars.update({'CC': cc, 'CXX': cxx})
            env_build.configure(vars=env_build_vars)
            env_build.make(vars=env_build_vars)
            env_build.install(vars=env_build_vars)
    def build(self):
        if self.settings.os_build == 'Windows':
            self._build_vs()
        else:
            self._build_configure()
    def package(self):
        self.copy(pattern="LICENSE", src='sources', dst="licenses")
        if self.settings.os_build == 'Windows':
            self.copy(pattern='*.exe', src='sources', dst='bin', keep_path=False)
    def package_info(self):
        self.env_info.PATH.append(os.path.join(self.package_folder, 'bin'))
    def package_id(self):
        del self.info.settings.compiler