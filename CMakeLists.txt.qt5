# CMakeLists.txt
# 
# Copyright (C) 2024 Gustavo Bacagine
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.
#
# Written by Gustavo Bacagine <gustavo.bacagine@protonmail.com> in November 2024 
# 
# Description: Fork of cvs plugin for kdevelop
# 

cmake_minimum_required(VERSION 3.0)

project(cvs)

find_package(ECM "5.14.0" REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# I set it to fix compilation error in slackware 15
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KDevPlatform 5.2.40 REQUIRED)

# kdebugsettings file
if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES cvs.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES cvs.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcvs\")

########### next target ###############

ecm_qt_declare_logging_category(kdevcvs_LOG_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_CVS
    CATEGORY_NAME "kdevelop.plugins.cvs"
)

set(kdevcvs_PART_SRCS
    cvsplugin.cpp
    cvsmainview.cpp
    cvsgenericoutputview.cpp
    cvsjob.cpp
    cvsproxy.cpp
    editorsview.cpp
    commitdialog.cpp
    importmetadatawidget.cpp
    importdialog.cpp
    checkoutdialog.cpp
    cvsannotatejob.cpp
    cvslogjob.cpp
    cvsdiffjob.cpp
    cvsstatusjob.cpp
    ${kdevcvs_LOG_PART_SRCS}
)

set(kdevcvs_PART_UI
    cvsmainview.ui
    cvsgenericoutputview.ui
    editorsview.ui
    commitdialog.ui
    importmetadatawidget.ui
    checkoutdialog.ui
)

ki18n_wrap_ui(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})

qt5_add_resources(kdevcvs_PART_SRCS kdevcvs.qrc)
kdevplatform_add_plugin(kdevcvs JSON kdevcvs.json SOURCES ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
    KF5::KIOWidgets
    KF5::Parts
    KDev::Util
    KDev::Interfaces
    KDev::Vcs
    KDev::Project
    KDev::Language
)

# TODO: Fix the tests compilation error
#if(BUILD_TESTING)
#  add_subdirectory(tests)
#endif()

