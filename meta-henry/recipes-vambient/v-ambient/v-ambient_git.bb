SUMMARY = "Virtual Ambient Sensor for OpenBMC"
DESCRIPTION = "A virtual sensor daemon using sdbusplus asio"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

DEPENDS += " \
    sdbusplus \
    boost \
    nlohmann-json \
"

inherit cmake systemd pkgconfig

S = "${UNPACKDIR}/sources"

SRC_URI = "file://v-ambient-assoc.cpp;subdir=sources \
           file://CMakeLists.txt;subdir=sources \
           file://v-ambient.service;subdir=sources"

SYSTEMD_SERVICE:${PN} = "v-ambient.service"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/v-ambient.service ${D}${systemd_system_unitdir}
}

