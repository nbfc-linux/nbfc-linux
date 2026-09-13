# Maintainer: Benjamin Abendroth <braph93@gmx.de>
# Contributor: Francesco Minnocci <ascoli dot minnocci at gmail dot com>

pkgname=nbfc-linux
pkgver=0.4.1
pkgrel=1
pkgdesc="Lightweight C port of NoteBook FanControl (no Mono required)"
arch=('i686' 'x86_64')
url="https://github.com/nbfc-linux/nbfc-linux"
license=('GPL-3.0-only')
conflicts=('nbfc')
provides=('nbfc')
depends=('curl' 'openssl' 'acpica' 'acpi_call')
makedepends=('curl' 'openssl')
source=("$pkgname-$pkgver.tar.gz::https://github.com/nbfc-linux/nbfc-linux/archive/refs/tags/${pkgver}.tar.gz")
sha256sums=('fd395f49da23a5cfc1ac6a11047f82cc588fd47b3450f678f2462090a53e9931')

build() {
	cd "${srcdir}/${pkgname}-${pkgver}"

  ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --bindir=/usr/bin && make
}

package() {
	cd "${srcdir}/${pkgname}-${pkgver}"

  make DESTDIR="${pkgdir}" install
}

# vim:set ts=2 sw=2 et:
