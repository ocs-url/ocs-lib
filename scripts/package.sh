#!/bin/bash

PKGNAME='ocs-manager'

USER='pkgbuilder'

SCRIPT="${0}"

PROJDIR="$(cd "$(dirname "${0}")/../" && pwd)"

BUILDDIR="${PROJDIR}/build_${PKGNAME}"

ci_appimage() { # docker-image: ubuntu:14.04
    apt update -qq
    apt -y install curl git
    #apt -y install build-essential qt5-default libqt5websockets5-dev
    #apt -y install cmake libssl-dev libcurl3 libcurl3-gnutls libcurl4-gnutls-dev libxpm-dev
    apt -y install libssl1.0.0 zlib1g unzip p7zip

    apt -y install software-properties-common
    add-apt-repository -y ppa:beineri/opt-qt593-trusty
    echo 'deb http://download.opensuse.org/repositories/home:/TheAssassin:/AppImageLibraries/xUbuntu_14.04/ /' > /etc/apt/sources.list.d/curl-httponly.list
    curl -fsSL https://download.opensuse.org/repositories/home:TheAssassin:AppImageLibraries/xUbuntu_14.04/Release.key | apt-key add -
    apt update -qq

    apt -y install build-essential libfontconfig1 mesa-common-dev libglu1-mesa-dev qt59base qt59websockets
    curl -fsSL https://cmake.org/files/v3.10/cmake-3.10.0-rc5-Linux-x86_64.tar.gz | tar -xz --strip-components=1 -C /
    apt -y install libssl-dev libcurl3 libcurl3-gnutls libcurl4-gnutls-dev libxpm-dev

    useradd -m ${USER}
    chown -R ${USER} "${PROJDIR}"

    su -c "export HOME=/home/${USER} && source /opt/qt59/bin/qt59-env.sh && sh "${SCRIPT}" build_appimage" ${USER}

    transfer_file "$(find "${BUILDDIR}" -type f -name "${PKGNAME}*.AppImage")"
}

build_appimage() {
    rm -rf "${BUILDDIR}"
    mkdir -p "${BUILDDIR}"
    export_srcarchive "${BUILDDIR}/${PKGNAME}.tar.gz"
    tar -xzf "${BUILDDIR}/${PKGNAME}.tar.gz" -C "${BUILDDIR}"
    cp "${PROJDIR}/pkg/appimage/appimage.sh" "${BUILDDIR}/${PKGNAME}"
    cd "${BUILDDIR}/${PKGNAME}"
    sh appimage.sh
}

export_srcarchive() {
    if [ "${1}" ]; then
        $(cd "${PROJDIR}" && git archive --prefix="${PKGNAME}/" --output="${1}" HEAD)
    fi
}

transfer_file() {
    if [ -f "${1}" ]; then
        filename="$(basename "${1}")"
        transferlog="${PROJDIR}/transfer.log"
        echo "Uploading ${filename}" >> "${transferlog}"
        curl -fsSL -T "${1}" "https://transfer.sh/${filename}" >> "${transferlog}"
        echo '' >> "${transferlog}"
    fi
}

if [ "${1}" ]; then
    ${1}
fi
