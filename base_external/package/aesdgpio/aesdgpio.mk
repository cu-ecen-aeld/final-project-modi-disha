
##############################################################
#
# AESDGPIO
#
##############################################################

AESDGPIO_VERSION=5f8290b94c2a674c4a2d28425f9f2eb794b7a87b
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESDGPIO_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
AESDGPIO_SITE_METHOD = git

define AESDGPIO_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/aesdgpio all
endef

define AESDGPIO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/aesdgpio/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
