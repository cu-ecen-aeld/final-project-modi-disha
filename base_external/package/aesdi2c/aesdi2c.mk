
##############################################################
#
# AESDI2C-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
AESDI2C_VERSION = f241688dc26b9e8d1a38af1ae8843430ee9ea376


# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESDI2C_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
AESDI2C_SITE_METHOD = git

define AESDI2C_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/aesdi2c all
endef

define AESDI2C_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/aesdi2c/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
