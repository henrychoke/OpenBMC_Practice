SUMMARY = "Debug Crash Exercise"
SECTION = "examples"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit cmake

S = "${UNPACKDIR}/sources"

SRC_URI = "file://debugcrash.cpp;subdir=sources \
           file://CMakeLists.txt;subdir=sources \
           file://debugcrash.service;subdir=sources"

SYSTEMD_SERVICE:${PN} = "debugcrash.service"

#INHIBIT_PACKAGE_STRIP = "1"
#INHIBIT_SYSROOT_STRIP = "1"
#INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/debugcrash.service ${D}${systemd_system_unitdir}
}

FILES:${PN} += "${systemd_system_unitdir}/debugcrash.service"

SELECTED_OPTIMIZATION = "-Og -g"
