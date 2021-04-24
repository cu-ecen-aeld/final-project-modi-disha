
##############################################################
#
# AESDLCD
#
##############################################################

AESDLCD_VERSION=fba861315b21d5ed24bdb6d2a839a8d3e4742a74
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESDLCD_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
AESDLCD_SITE_METHOD = git

define AESDLCD_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/aesdlcd all
endef

define AESDLCD_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/aesdlcd/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
