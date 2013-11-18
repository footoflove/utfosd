###############################################################################
#                                                                             #
# User Application Make File                                                  #
#                                                                             #
# Author: Paul Rayman, 01/05/04                                               #
#                                                                             #
# Copyright, 2003 Conexant Systems. All Rights Reserved                       #
#                                                                             #
###############################################################################

######################################################################
# File list. These files make up this driver.                        #
######################################################################

GENERAL_C_FILES = UTFZOrderManage.C UTFWndFunc.C UTFVideo.C UTFTypeManage.C \
				UTFText.C UTFScrollBar.C UTFRect.C UTFProgress.C UTFPalette.C \
				UTFMsgBox.C UTFMenu.C UTFMemory.C UTFLRList.C UTFList.C \
				UTFImage.C UTFGraphPort.C UTFFont.C UTFEnglishRules.C \
				UTFEdit.C UTFDrawText.C UTFDrawAPI.C UTFDialog.C UTFButton.C \
				UTFCheckBox.C UTFArabicRules.C UTFCHNRules.C UTFBIG5Rules.C \
				UTFRUSRules.C UTFGifDraw.C UTFHindiRules.C UTFTamilRules.C \
				UTFMalayalamRules.C UTFKannadaRules.C UTFTimer.C UTFThaiRules.C \
				UTFMath.C UTFJpegCommon.C UTFJpegDC.C UTFJpegDecode.C UTFDCImage.C \
				UTFjpeg.C
				
EXTRA_INCLUDE_DIRS = $(EXTRA_INCLUDE_DIRS) $(SABINE_ROOT)\BSP\CNXT_VIRGO\INCLUDE  
EXTRA_INCLUDE_DIRS = $(EXTRA_INCLUDE_DIRS) $(SABINE_ROOT)\BSP\CNXT_VIRGO\demux 				
EXTRA_INCLUDE_DIRS = $(EXTRA_INCLUDE_DIRS) $(SABINE_ROOT)\DG_AL\AL_VIRGO\INCLUDE
EXTRA_INCLUDE_DIRS = $(EXTRA_INCLUDE_DIRS) $(SABINE_ROOT)\DG_MW\include