
##############################################################
#
# AESD-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
SOCKETCOMM_VERSION=ec13c5df39bfade61515758bcb3df906b6861c38
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
SOCKETCOMM_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
SOCKETCOMM_SITE_METHOD = git

define SOCKETCOMM_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/Client all
endef

# TODO add your writer, finder and tester utilities/scripts to the installation steps below
define SOCKETCOMM_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/Client/* $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
