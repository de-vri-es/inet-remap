pkgname=inet-remap
pkgver=0.1
pkgrel=1
url='https://github.com/de-vri-es/inet-remap'
license=('GPLv3')
arch=('x86_64' 'i686')
source=("inet_remap-$pkgver.tar.gz")

sha512sums=("SKIP")

prepare() {
	cd "$srcdir/inet_remap-$pkgver"
	./configure --prefix=/usr
}

build() {
	cd "$srcdir/inet_remap-$pkgver"
	make
}

package() {
	cd "$srcdir/inet_remap-$pkgver"
	make DESTDIR="$pkgdir/" install
}
