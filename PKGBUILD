# Maintainer: Michael Scott <support@s4gadgets.com>

pkgname=s4gadgets-vga-csi2-driver
pkgver=1.0.0
pkgrel=1
pkgdesc="S4-Gadgets AD9984A + TC358748 V4L2 Pipeline for Raspberry Pi (Arch Linux)"
arch=('armv7h' 'aarch64')
url="https://s4gadgets.com/s4gadgets-vga-csi2-driver"
license=('GPL')
depends=('base-devel' 'linux-headers' 'raspberrypi-firmware' 'v4l-utils' 'i2c-tools')
makedepends=('git')
install=${pkgname}.install
source=(
  "install.sh"
  "s4_ad9984a.c"
  "s4_tc358748.c"
  "s4_ad9984a.h"
  "s4_tc358748.h"
  "99-s4gadgets.rules"
  "s4gadgets-v4l2.service"
)
sha256sums=('SKIP' 'SKIP' 'SKIP' 'SKIP' 'SKIP' 'SKIP' 'SKIP')

package() {
  install -Dm755 install.sh "$pkgdir/usr/bin/s4gadgets-v4l2-install"
  install -Dm644 s4_ad9984a.c "$pkgdir/usr/src/s4_ad9984a/s4_ad9984a.c"
  install -Dm644 s4_ad9984a.h "$pkgdir/usr/src/s4_ad9984a/s4_ad9984a.h"
  install -Dm644 s4_tc358748.c "$pkgdir/usr/src/s4_tc358748/s4_tc358748.c"
  install -Dm644 s4_tc358748.h "$pkgdir/usr/src/s4_tc358748/s4_tc358748.h"
  install -Dm644 99-s4gadgets.rules "$pkgdir/usr/lib/udev/rules.d/99-s4gadgets.rules"
  install -Dm644 s4gadgets-v4l2.service "$pkgdir/usr/lib/systemd/system/s4gadgets-v4l2.service"
}
