EAPI=5

EGIT_REPO_URI="https://github.com/mosra/magnum-extras.git"

inherit cmake-utils git-r3

DESCRIPTION="Extras for the Magnum C++11/C++14 graphics engine"
HOMEPAGE="https://magnum.graphics"

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

RDEPEND="
	dev-libs/magnum
	dev-libs/magnum-plugins
"
DEPEND="${RDEPEND}"

src_configure() {
	# general configuration
	local mycmakeargs=(
		-DCMAKE_INSTALL_PREFIX="${EPREFIX}/usr"
		-DCMAKE_BUILD_TYPE=Release
		-DMAGNUM_WITH_PLAYER=ON
		-DMAGNUM_WITH_UI=ON
		-DMAGNUM_WITH_UI_GALLERY=ON
	)
	cmake-utils_src_configure
}

# kate: replace-tabs off;
