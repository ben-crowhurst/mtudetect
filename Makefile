include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-mtudetect
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS := 

include $(GLUONDIR)/include/package.mk

define Package/gluon-mtudetect
  SECTION:=gluon
  CATEGORY:=Gluon
  TITLE:=Detects the MTU by pinging a given host
  DEPENDS:=
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/gluon-mtudetect/install
	$(CP) ./files/* $(1)/

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/mtudetect $(1)/usr/bin/
endef

define Package/gluon-mtudetect/postinst
endef

$(eval $(call BuildPackage,gluon-mtudetect))
