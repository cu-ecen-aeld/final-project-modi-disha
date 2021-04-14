
##############################################################
#
# GPIOMODULE-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
GPIOMODULE_VERSION = 0f0dda53107aced36b42d9e5b58f909050a4e4b7


# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
GPIOMODULE_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
GPIOMODULE_SITE_METHOD = git
GPIOMODULE_MODULE_SUBDIRS = aesd-gpio-driver/

$(eval $(kernel-module))
$(eval $(generic-package))
