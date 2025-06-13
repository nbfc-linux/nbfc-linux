# Maintainer: Benjamin Abendroth <braph93@gmx.de>
# Contributor: Francesco Minnocci <ascoli dot minnocci at gmail dot com>

pkgname=nbfc-linux
pkgver=0.3.19
pkgrel=1
pkgdesc="Lightweight C port of NoteBook FanControl (no Mono required)"
arch=('i686' 'x86_64')
url="https://github.com/nbfc-linux/nbfc-linux"
license=('GPL-3.0-only')
conflicts=('nbfc')
provides=('nbfc')
depends=('curl' 'openssl')
makedepends=('curl' 'openssl')
source=("$pkgname-$pkgver.tar.gz::https://github.com/nbfc-linux/nbfc-linux/archive/refs/tags/${pkgver}.tar.gz")
sha256sums=('b36f5851100bb3493a7c2957b58acd0e163a7781431c386ccd3b3de9318c6223')

build() {
	cd "${srcdir}/${pkgname}-${pkgver}"

  ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --bindir=/usr/bin && make
}

package() {
	cd "${srcdir}/${pkgname}-${pkgver}"

  make DESTDIR="${pkgdir}" install
}
# vim:set ts=2 sw=2 et:
