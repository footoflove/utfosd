
#ifndef _UTF_JPEG_DECODE_H
#define _UTF_JPEG_DECODE_H


// FLAGS ////////////////////////////////////////////////////////////////////////////////////////
// ������� ������� ������
#define jpegAPP0Present			0x0001	 // JFIF ���� ������������
#define jpegCOMPresent			0x0002	 // COM ���� ������������
#define jpegSOIPresent			0x0004	 // SOI ���� ������������
#define jpegEOIPresent			0x0008	 // EOI ���� ������������
#define jpegPreviewPresent		0x0010	 // � ������� ����� JFIF ������������ ������������� ��� ������
#define jpegSOF0Present			0x0020	 // SOF0 ���� ������������
#define jpegSOF2Present			0x0040	 // SOF0 ���� ������������
#define jpegDRIPresent			0x0080	 // DRI ���� ������������
#define jpegDQTPresent			0x0100	 // DQT ���� ������������
#define jpegDHTPresent			0x0200	 // DHT ���� ������������
#define jpegSOSPresent 			0x0400	 // SOS ���� ������������
#define jpegRSTxPresent 		0x0800	 // RSTx ���� ������������
#define jpegAPP1Present			0x1000	 // EXIF ���� ������������

// ��� jpeg
#define jpegSingleScan			0x10000	 // ����������������, ���� ����
#define jpegMultipleScan		0x20000	 // ����������������, ����� ������
#define jpegProgressive			0x40000	 // �������������

#define SET_FLAG( jpg, flag )	(jpg->blockPresentField |= flag)
#define CHK_FLAG( jpg, flag )	((jpg->blockPresentField & flag) == flag)
#define CLR_FLAGS( jpg )		(jpg->blockPresentField = 0)
#define GET_FLAGS( jpg )		(jpg->blockPresentField)

// ������� JPEG
#define jpegTEM    0xff01   // ������ �������� ������ �������������, ����� ���� ���������������
#define jpegSOF0   0xffc0   // ������ ����� (Baseline JPEG)
#define jpegSOF1   0xffc1   // == SOF0
#define jpegSOF2   0xffc2   // �������������
#define jpegSOF3   0xffc3   // 
#define jpegDHT    0xffc4   // ����������� ������� ��������
#define jpegSOF5   0xffc5   // 
#define jpegSOF6   0xffc6   // 
#define jpegSOF7   0xffc7   // 
#define jpegJPG    0xffc8   // ��������������/����������������� (�������� ������ �������������)
#define jpegSOF9   0xffc9   // ��� ��������������� �����������, ������ �� ��������������
#define jpegSOF10  0xffca   // ������ ����������������
#define jpegSOF11  0xffcb   // 
#define jpegDAC    0xffcc   // ����������� �������������� �������, ������ �� ��������������
#define jpegSOF13  0xffcd   // 
#define jpegSOF14  0xffce   // 
#define jpegSOF15  0xffcf   // 
#define jpegRST0   0xffd0   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST1   0xffd1   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST2   0xffd2   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST3   0xffd3   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST4   0xffd4   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST5   0xffd5   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST6   0xffd6   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegRST7   0xffd7   // RSTn ������������ ��� �������������, ����� ���� ���������������
#define jpegSOI    0xffd8   // ������ �����������
#define jpegEOI    0xffd9   // ����� �����������
#define jpegSOS    0xffda   // ������ ������������
#define jpegDQT    0xffdb   // ����������� ������� �����������
#define jpegDNL    0xffdc   // ����������� ���������� �����, ������ ����������������, ������������
#define jpegDRI    0xffdd   // ����������� ��������� �����������
#define jpegAPP0   0xffe0   // JFIF ������� ��������� ����������
#define jpegAPP1   0xffe1   // JFIF ������� ��������� ����������
#define jpegAPP2   0xffe2   // JFIF ������� ��������� ����������
#define jpegAPP3   0xffe3   // JFIF ������� ��������� ����������
#define jpegAPP4   0xffe4   // JFIF ������� ��������� ����������
#define jpegAPP5   0xffe5   // JFIF ������� ��������� ����������
#define jpegAPP6   0xffe6   // JFIF ������� ��������� ����������
#define jpegAPP7   0xffe7   // JFIF ������� ��������� ����������
#define jpegAPP8   0xffe8   // JFIF ������� ��������� ����������
#define jpegAPP9   0xffe9   // JFIF ������� ��������� ����������
#define jpegAPP10  0xffea   // JFIF ������� ��������� ����������
#define jpegAPP11  0xffeb   // JFIF ������� ��������� ����������
#define jpegAPP12  0xffec   // JFIF ������� ��������� ����������
#define jpegAPP13  0xffed   // JFIF ������� ��������� ����������
#define jpegAPP14  0xffee   // JFIF ������� ��������� ����������
#define jpegAPP15  0xffef   // ������������
#define jpegCOM    0xfffe   // �����������

////////////////////////////////////////////////////////////////
// 
#define resNull							0x00000000	// null
#define resOk							0x00000001	// ok
#define resFail							0x00000002	// fail
#define resBreak						0x00000003	// user break
#define resNoMem						0x00000030	// ��� ������

// storage 
#define resEOB							0x00000120	// ��������� ����� �����
#define resBStOutOfRange				0x00000121	// ������������� ������ ����� �� ������� ��������� (�����)
#define resBStUncorrectPos				0x00000122	// ������������ �������
#define resBStCannotOpen				0x00000123	// �� ������� ������� �������� (db/file)

// d.c.image
#define resInvalidDrawCtxForAttach		0x00000200	// ���������� ����������� draw context � �����������
#define resUncompatDrawCtxForAttach		0x00000201	// ��������������� image � draw context
#define resInvalidImageCoord			0x00000202	// ������������ ���������� �����������
#define resNoLockedImage				0x00000203	// ����������� �� ���� ��������
#define resNoRefreshCtxImage			0x00000204	// ����������� �� ����� ��������, ��������� � ��� ��������
#define resNoResizeImage				0x00000205	// ����������� �� ����� ���� resize
#define resImageCrashed					0x00000206	// ����������� ���������! (��,��������� ��������) 
#define resImageCreated					0x00000207	// ����������� ��� �������
#define resImageNoCreated				0x00000208	// ����������� �� �������
#define resImageAttached				0x00000209	// ����������� ���� �����������
#define resImageDettached				0x0000020a	// ����������� ���� dettach
#define resImageInDynamic				0x0000020b	// ����������� �������� � ���.������
#define resImageInStatic				0x0000020c	// ����������� �������� � ��.������
#define resImageInDynamicOrStatic		0x0000020d	// ����������� �������� � ���. ��� ��.������
#define resImageCannotRotate			0x0000020e	// ����������� �� ����� ���� ���������
#define resImageCannotResize			0x0000020f	// ����������� �� ����� ���� resize
#define resInvalidLenSrcForResize		0x00000210	// ����������� ��� resize �������� ������� ���� �����
#define resCannotPackInvalidDB			0x00000211	// ����������� �� ����� ���� ����������; �� �� ����� ���� �������
#define resCannotUnpackInvalidDB		0x00000212	// ����������� �� ����� ���� �����������; �� �� ���������� ��� �� ����� ���� �������
#define resCannotPackExistDB			0x00000213	// ����������� �� ����� ���� ����������; �� ��� ���������� 

// jpeg
#define resDoubledBlock					0x00001000	// ��������������: ���� ��� ���������� �����
#define resIgnoredBlock					0x00001001	// ��������������: ���� �����������
#define resInvalidBlock					0x00001002	// ��������� ������: ������������ ����
#define resNoJPEG						0x00001003	// ��������� ������: �� JPEG ����
#define resNoJFIF						0x00001004	// ��������� ������: �� JFIF ����
#define resMayBeNoJFIF					0x00001005	// ��������������: �������� �� JFIF ����
#define resLossesData					0x00001006	// ��������������: ������ � ����� �� ������������� ������ �������������
#define resUnknownVerJFIF				0x00001007	// ��������� ������: ����������� ������ JFIF �����
#define resUnsupportVerJFIF				0x00001008	// ��������� ������: ���������������� ������ JFIF �����
#define resUnsupportVerEXIF				0x00001009	// ��������� ������: ���������������� ������ EXIF �����
#define resUnsupportFormat				0x0000100a	// ��������� ������: ���������������� ������
#define resUnsupportPrecision			0x0000100b	// ��������� ������: ���������������� �������� ����� (�� 8���/����)
#define resInvalidWidthHeight			0x0000100c	// ��������� ������: ������������ ������ �������� (==0)
#define resUnsupportColorComp			0x0000100d	// ��������� ������: ���������������� ����� ��������� ����� (������ 1-gray ��� 3-rgb)
#define resInvalidLenDRI				0x0000100e	// ��������� ������: ������������ ������ ����� DRI
#define resInvalidSizeQuantTable		0x0000100f	// ��������� ������: ������������ ������ ����� DQT
#define resInvalidNumQuantTable			0x00001010	// ��������������: �������� � �������� ������ �������������
#define resInvalidSizeHufTable			0x00001011	// ��������� ������: ������������ ������ ����� DHT
#define resInvalidNumHufTable			0x00001012	// ��������������: �������� � �������� ������ ��������
#define resInvalidNumScanComp			0x00001013	// ��������� ������: ������������ ����� ����������� ������������
#define resInvalidSizeSOSBlock			0x00001014	// ��������� ������: ������������ ������ ����� SOS
#define resUnadmitBlockId				0x00001015	// ��������������: ������������ id ����� (��������, 0x00000003)
#define res16bitQuantTable				0x00001016	// ��������� ������: � 16������� ��������� ����������� ���� �� ��������
#define resInvalidRSTxMarker			0x00001017	// ��������� ������: ������������ ������������������ �������� �����������
#define resDehufCorrupt					0x00001018	// ��������� ������: �� ������� ������������ �� �������� ���� DU (������ ����� ����� ������)
#define resBlockFamilyCorrupt			0x00001019	// ��������� ������: �����, ����������� ��� ���������� �����������
#define resDoubledSOSBlock				0x0000101a	// ��������� ������: ���� SOS ��� ���������� �����
#define resJPEGCorrupt					0x0000101b	// ��������� ������: JPEG '�����' (��������, � ����� ����� ��� ������� 0x000000d9)
#define resHufCodeNotFound				0x0000101c	// ��������� ������: ��� ���������� ��� ��������� ������ ��� �� ��� ������ ��������������� ���
#define resScalefactorNotSupported		0x0000101d	// ��������� ������: ����� �������� �������� ��� �����������
#define resScaleUnusable				0x0000101e	// ��������� ������: ����� ������� ��������� (<1 � 8) �������� ��� ����������� - ����� �� �������������� ��������, � ������������� ������ DC
#define resWantOfBlocks					0x0000101f	// ��������� ������: �� ��� �������� ������������, ���������� ��������
#define resStartOfScan					0x00001020	// ��������������: ������ ������������
#define resEndOfJpeg					0x00001021	// ��������������: ����� jpeg
#define resUnsupportedDiskretVal		0x00001022	// ��������� ������: ���������������� �������� �������� > 2
#define resSpecSelectionError			0x00001023	// ��������� ������: � ������������ ����������
#define resSucesApproxError				0x00001024	// ��������� ������: � ���������������� �����������
#define resPrgScanDataError				0x00001025	// ��������� ������: � ������ �������������� �����
#define resPrgDUEOB						0x00001026	// ��������������: ������ end-of-band
#define resPrgUncorrectCompNum			0x00001027	// ��������������: ������������ ����� ��������� � ������������� �����
#define resExifCorrupt					0x00001028	// ��������� ������: ������������ EXIF block
#define resNoExif						0x00001029	// ��������� ������: �� EXIF block
#define resExifUnsupportByteAlign		0x0000102a	// ��������� ������: ���������������� ������ ���������� ������
#define resHufTooManyCodes				0x0000102b	// ��������� ������: ��� ����������� �������� ����� �����
#define resHufMissingCodeSize			0x0000102c	// ��������� ������: ��� ����������� 
#define resHufDuplicateVal				0x0000102d	// ��������� ������: ��� ����������� 
#define resHufMissingValue				0x0000102e	// ��������� ������: ��� ����������� 
#define resHufInvalidRange				0x0000102f	// ��������� ������: ��� ����������� 
#define resHufDuplicateCode				0x00001030	// ��������� ������: ��� ����������� 

#ifndef LOPART 

#define LOPART(x)	((UInt8)((UInt8)(x) & 0x0f)) 
#define HIPART(x)	((UInt8)((UInt8)(x) >> 0x04)) 

#endif

#define jpegSegmentSize		0xfff0					// ������ �������� jpeg (��������������), �� 65��

#define jpegReserveSize		0x1000					// ������ ������ (�������� �� jpegReserveSize ���� ������)

#define jpegAPP0Sign		0x4a464946				// JFIF
#define jpegExifSign		0x45786966				// Exif
#define jpegEXIFSign		0x45584946				// EXIF
#define jpegAPP0SignXX		0x4a465858				// JFXX

#define jpegMaxBlockSize	0xffff					// ������������ ����� �����
#define jpegBlockSign		0xff					// ��������� ������ �����
#define jpegMaxTableNum		4//3						// ������ �������������� ������ ��� ������� ����������� Y,Cr,Cb, � ������ ����� ���� ������ - CMYK
#define jpegMaxHufTableNum	4//3						// ������ �������������� ������ 4 ������� ����� Huffman
#define jpegMaxDiscretVal	10						// ������������ �������� �������-�� ��� 4:4:1 -> 4 + 4 + 1 + ����� (������ 4:1:1, 1:1:1)
#define jpegMaxDUinMCU		4						// ����. ����� du � ������ mcu
#define jpegMaxComponentNum 4						// ������������ ����� �����������
#define jpegMinHVSize		16						// ����������� ������ ��������, � ������� ����� ��������

#define jpegDUHVSize		8						// ������ ����� DU �� ������ �������
#define jpegMCUMaxHSize		2 * jpegDUHVSize		// ������ ����� MCU �� ����������� � ��������
#define jpegMCUMaxVSize		2 * jpegDUHVSize		// ������ ����� MCU �� ��������� � ��������
#define jpegMCUMaxSize		jpegMCUMaxHSize * jpegMCUMaxVSize

// id ��������� ����������� (CMYK �� �� ������������)
#define compY	0			// �������
#define compCb	1			// ������������� ��� ������
#define compCr	2			// ������������� ��� ��������
#define compCc	1			// cyan
#define compCm	2			// magenta
#define compCy	3			// yellow

// id ��������� ����������� (JFIF ��������)
#define compYjf		1			// �������
#define compCbjf	2			// ������������� ��� ������
#define compCrjf	3			// ������������� ��� ��������
#define compCcfp	2			// cyan
#define compCmjp	3			// magenta
#define compCyjp	4			// yellow

// colorSpace
#define asYCbCr		0
#define asY			asYCbCr
#define asYCcCe		1


#define jpegUnZigzagTable	{0, 1, 8,16, 9, 2, 3,10,17,24,32,25,18,11,4, 5,12,19,26,33,40, \
							48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22, \
							15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63 }
#define jpegZigzagTable		{0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,42,3,8,12,17,25,30,41,43, \
							9,11,18,24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,33,38,46, \
							51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63 }
#define jpegCosScaledTable	{32, 23, 24, 27, 32, 40, 59, 115, \
							23, 16, 17, 19, 23, 29, 42, 83, \
							24, 17, 18, 20, 24, 31, 45, 88, \
							27, 19, 20, 23, 27, 34, 50, 98,  \
							32, 23, 24, 27, 32, 40, 59, 115, \
							40, 29, 31, 34, 40, 51, 75, 147, \
							59, 42, 45, 50, 59, 75, 109, 214, \
							115,83, 88, 98, 115,147,214, 420 }
#define jpegCosScaledTableForRGB16	\
							{32, 23, 24, 27, 32, 40, 59, 115, \
							23, 16, 17, 19, 23, 29, 42, 83, \
							24, 17, 18, 20, 24, 31, 45, 88, \
							27, 19, 20, 23, 27, 34, 50, 98,  \
							32, 23, 24, 27, 0,0,0,0, \
							40, 29, 31, 34, 0,0,0,0, \
							59, 42, 45, 50, 0,0,0,0, \
							115,83, 88, 98, 0,0,0,0 }
#define jpegCosScaledTableForRGB16x4	\
							{32, 23, 24, 27, 0,  0,  0,  0, \
							23, 16, 17, 19, 0, 0, 0, 0, \
							24, 17, 18, 20, 0, 0, 0, 0, \
							27, 19, 20, 23, 0, 0,  0, 0,  \
							0,  0,  0,  0,  0,  0,  0,  0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0 }
#define jpegCosScaledTableForRGB16x2	\
							{32, 23, 0, 0, 0,  0,  0,  0, \
							23, 16, 0,  0, 0, 0, 0, 0, \
							0,  0,  0,  0, 0, 0, 0, 0, \
							0,  0,  0,  0, 0, 0,  0, 0,  \
							0,  0,  0,  0,  0,  0,  0,  0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0 }
#define jpegCosScaledTableForRGB16xT	\
							{32, 0, 0, 0, 0,  0,  0,  0, \
							0,  0,  0,  0, 0, 0, 0, 0, \
							0,  0,  0,  0, 0, 0, 0, 0, \
							0,  0,  0,  0, 0, 0,  0, 0,  \
							0,  0,  0,  0,  0,  0,  0,  0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0, \
							0,  0,  0,  0,  0,  0, 0, 0 }
#define jpegCosScaledMaskForRGB16	\
						   {1, 1, 1, 1, 1, 1, 1, 1, \
							1, 1, 1, 1, 1, 1, 1, 1, \
							1, 1, 1, 1, 1, 1, 1, 1, \
							1, 1, 1, 1, 1, 1, 1, 1,  \
							1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0 }
#define jpegCosScaledMaskForRGB16x4	\
						   {1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0, \
							1, 1, 1, 1, 0, 0, 0, 0,  \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0 }
#define jpegCosScaledMaskForRGB16x2	\
						   {1, 1, 0, 0, 0, 0, 0, 0, \
							1, 1, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0,  \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0 }
#define jpegCosScaledMaskForRGB16xT	\
						   {1, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0,  \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0, \
							0, 0, 0, 0, 0, 0, 0, 0 }
							
#define jpegCbConvertTable \
{ -11264, -11176, -11088, -11000, -10912, -10824, -10736, -10648, -10560, -10472, -10384, -10296, -10208, -10120, -10032,  -9944, \
  -9856,  -9768,  -9680,  -9592,  -9504,  -9416,  -9328,  -9240,  -9152,  -9064,  -8976,  -8888,  -8800,  -8712,  -8624,  -8536, \
  -8448,  -8360,  -8272,  -8184,  -8096,  -8008,  -7920,  -7832,  -7744,  -7656,  -7568,  -7480,  -7392,  -7304,  -7216,  -7128, \
  -7040,  -6952,  -6864,  -6776,  -6688,  -6600,  -6512,  -6424,  -6336,  -6248,  -6160,  -6072,  -5984,  -5896,  -5808,  -5720, \
  -5632,  -5544,  -5456,  -5368,  -5280,  -5192,  -5104,  -5016,  -4928,  -4840,  -4752,  -4664,  -4576,  -4488,  -4400,  -4312, \
  -4224,  -4136,  -4048,  -3960,  -3872,  -3784,  -3696,  -3608,  -3520,  -3432,  -3344,  -3256,  -3168,  -3080,  -2992,  -2904, \
  -2816,  -2728,  -2640,  -2552,  -2464,  -2376,  -2288,  -2200,  -2112,  -2024,  -1936,  -1848,  -1760,  -1672,  -1584,  -1496, \
  -1408,  -1320,  -1232,  -1144,  -1056,   -968,   -880,   -792,   -704,   -616,   -528,   -440,   -352,   -264,   -176,    -88, \
      0,     88,    176,    264,    352,    440,    528,    616,    704,    792,    880,    968,   1056,   1144,   1232,   1320, \
   1408,   1496,   1584,   1672,   1760,   1848,   1936,   2024,   2112,   2200,   2288,   2376,   2464,   2552,   2640,   2728, \
   2816,   2904,   2992,   3080,   3168,   3256,   3344,   3432,   3520,   3608,   3696,   3784,   3872,   3960,   4048,   4136, \
   4224,   4312,   4400,   4488,   4576,   4664,   4752,   4840,   4928,   5016,   5104,   5192,   5280,   5368,   5456,   5544, \
   5632,   5720,   5808,   5896,   5984,   6072,   6160,   6248,   6336,   6424,   6512,   6600,   6688,   6776,   6864,   6952, \
   7040,   7128,   7216,   7304,   7392,   7480,   7568,   7656,   7744,   7832,   7920,   8008,   8096,   8184,   8272,   8360, \
   8448,   8536,   8624,   8712,   8800,   8888,   8976,   9064,   9152,   9240,   9328,   9416,   9504,   9592,   9680,   9768, \
   9856,   9944,  10032,  10120,  10208,  10296,  10384,  10472,  10560,  10648,  10736,  10824,  10912,  11000,  11088,  11176 }
   
#define jpegCrConvertTable \
{ -23552, -23368, -23184, -23000, -22816, -22632, -22448, -22264, -22080, -21896, -21712, -21528, -21344, -21160, -20976, -20792, \
 -20608, -20424, -20240, -20056, -19872, -19688, -19504, -19320, -19136, -18952, -18768, -18584, -18400, -18216, -18032, -17848, \
 -17664, -17480, -17296, -17112, -16928, -16744, -16560, -16376, -16192, -16008, -15824, -15640, -15456, -15272, -15088, -14904, \
 -14720, -14536, -14352, -14168, -13984, -13800, -13616, -13432, -13248, -13064, -12880, -12696, -12512, -12328, -12144, -11960, \
 -11776, -11592, -11408, -11224, -11040, -10856, -10672, -10488, -10304, -10120,  -9936,  -9752,  -9568,  -9384,  -9200,  -9016, \
  -8832,  -8648,  -8464,  -8280,  -8096,  -7912,  -7728,  -7544,  -7360,  -7176,  -6992,  -6808,  -6624,  -6440,  -6256,  -6072, \
  -5888,  -5704,  -5520,  -5336,  -5152,  -4968,  -4784,  -4600,  -4416,  -4232,  -4048,  -3864,  -3680,  -3496,  -3312,  -3128, \
  -2944,  -2760,  -2576,  -2392,  -2208,  -2024,  -1840,  -1656,  -1472,  -1288,  -1104,   -920,   -736,   -552,   -368,   -184, \
      0,    184,    368,    552,    736,    920,   1104,   1288,   1472,   1656,   1840,   2024,   2208,   2392,   2576,   2760, \
   2944,   3128,   3312,   3496,   3680,   3864,   4048,   4232,   4416,   4600,   4784,   4968,   5152,   5336,   5520,   5704, \
   5888,   6072,   6256,   6440,   6624,   6808,   6992,   7176,   7360,   7544,   7728,   7912,   8096,   8280,   8464,   8648, \
   8832,   9016,   9200,   9384,   9568,   9752,   9936,  10120,  10304,  10488,  10672,  10856,  11040,  11224,  11408,  11592, \
  11776,  11960,  12144,  12328,  12512,  12696,  12880,  13064,  13248,  13432,  13616,  13800,  13984,  14168,  14352,  14536, \
  14720,  14904,  15088,  15272,  15456,  15640,  15824,  16008,  16192,  16376,  16560,  16744,  16928,  17112,  17296,  17480, \
  17664,  17848,  18032,  18216,  18400,  18584,  18768,  18952,  19136,  19320,  19504,  19688,  19872,  20056,  20240,  20424, \
  20608,  20792,  20976,  21160,  21344,  21528,  21712,  21896,  22080,  22264,  22448,  22632,  22816,  23000,  23184,  23368 } 							
							
#define NULLx8				{0,0,0,0,0,0,0,0}
#define NULLx64				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define jpegLastReserv		0x400


// Types /////////////////////////////////////////////////////////////////////////////////////////
//

typedef struct bitProType
{
	UInt32		regData;
	UInt32		regIndex;
	UInt8		*bufP;
	UInt8		*dataP;
	UInt8		*rstxP;
	UInt32		rstxF;

} bitProType;

typedef struct JPEG_HufTableType
{
	UInt32				dhtInfo;			// ���������� � ��������
	UInt32				dhtBits[16];		// ������ 16 � ����� �����
	UInt32				dhtCodes[256];		// ������ 256 �� ����� ���� �� �����������

} JPEG_HufTableType;

typedef struct JPEG_HufTreeType
{
	UInt16				hufCode[256];				// ������ 256 �� ����� ���� �� �����������
	UInt16				hufData[256];				// ������ 256 �� ����� ���� �� �����������
	UInt16				hufSize[256];				// ������ 256 �� ����� ���� �� �����������
	UInt32				hufLen;						// ����� �����

} JPEG_HufTreeType;

typedef struct JPEG_QuantTableType			// �� ����� 4-� ������ 0..3
{
	UInt32				quantInfo;	
	UInt16				quantTable[65];		// ���� ����� �������� ������ � 8������� ��������� �������������

} JPEG_QuantTableType;

typedef struct JPEG_InfoType
{
// ����� ���������� � JPEGe
	UInt32		width;							// �������� ������
	UInt32 		height;							// �������� ������
	UInt32		compNum;						// ����� ���������
	UInt32		bitPerUnit;						// ��� �� ������ ���������

	UInt32		numMCU;							// ����� ����� MCU
	UInt32		numMCUx;						// �� ��� �� �����������
	UInt32		numMCUy;						// �� ��� �� ���������

	UInt32		widthM;							// ������ � ������ ��������
	UInt32		heightM;						// ����������� �� ������� mcu

	UInt32		qntNum;							// ����� ����� ������ �����������
	UInt32		hufNum;							// ����� ����� ������ ��������

	UInt32		resetInterval;					// �������� �����������, ������������ �������� DRI	

} JPEG_InfoType;


typedef struct JPEG_MCUType
{
	UInt32		sizeX;							// ������ �� X �������������� ����� MCU (��������)
	UInt32		sizeY;							// ������ �� Y �������������� ����� MCU (��������)
	UInt32		sizeMCU;						// ������ ����� MCU (��������)

	UInt32		vMax;							// ����. ������� �� ���������
	UInt32		hMax;							// ����. ������� �� �����������
	UInt32		duInMcu;						// ����� ����� du � mcu

	UInt32 		compNumInScan;					// ����� ��������� � �����
	UInt32		compId[jpegMaxComponentNum];	// id ��������� � �����
	UInt32		compMcuId[jpegMaxComponentNum];	// id ��������� � mcu
	UInt32		seqMCU[jpegMaxComponentNum];	// ������ ����� MCU � ������ DU ��� ������� ����������
	
	UInt8		repVC[jpegMaxComponentNum];
	UInt8		repHC[jpegMaxComponentNum];
	UInt8		seqVC[jpegMaxComponentNum];
	UInt8		seqHC[jpegMaxComponentNum];
	
	Int32		DC[jpegMaxComponentNum];		// ����� ����� ������������� �������� DC ��� ������� YCbCr ����������
												// 0-� ������� �� ������������ ������ ��� ��������, �.�. ���.����� ���������� - 1
	UInt32 		compPos[jpegMaxComponentNum];	// ������ ��������� ������� � ������� ������ ��� ������� ����������
	UInt32		numHufTblDC[jpegMaxDiscretVal];	// ������ ������ �������� ��� DC ��������� [0] - ��� Y, [1] - ��� Cb, [2] - ��� Cr
	UInt32		numHufTblAC[jpegMaxDiscretVal];	// ������ ������ �������� ��� AC ��������� [0] - ��� Y, [1] - ��� Cb, [2] - ��� Cr
	UInt32		numQuantTbl[jpegMaxDiscretVal];	// ������ ������ ����������� ��������� [0] - ��� Y, [1] - ��� Cb, [2] - ��� Cr

} JPEG_MCUType;


typedef struct JPEG_ScanType
{
	UInt32				scanIndex;					// ������� ������� � �����
	UInt32				numScans;					// ����� ����� ������
	UInt32				scanSSS;					// Spectral Selection Start
	UInt32				scanSSE;					// Spectral Selection End
	UInt32				scanSAH;					// Sucessive Approximation High
	UInt32				scanSAL;					// Sucessive Approximation Low
	
// ������ ������ ����������� � �������� (�������������)
	JPEG_QuantTableType	quantTable[jpegMaxTableNum];			// ����� ��� ������� �����������
	JPEG_HufTreeType	hufDCTree[jpegMaxHufTableNum];			// ������ �������� - � ������� 3*256 ����
	JPEG_HufTreeType	hufACTree[jpegMaxHufTableNum];			// ������ �������� - � ������� 3*256 ����

} JPEG_ScanType;

typedef struct JPEG_ThumbType
{
	UInt32				thumbnail;				// 0 - no, 1 - jpg, 2 - 8bit, 3 - rgb
	UInt32				thumbnailW;
	UInt32				thumbnailH;
	UInt32  			thumbnailIdx;	
	
} JPEG_ThumbType, * JPEG_ThumbPtr;

typedef struct JPEG_FormatType
{
	// common part
	UInt32				len;					// ����� ���������
	UInt32				width;					// ������ �����������
	UInt32				height;					// ������ �����������
	UInt32				bitPerUnit;				// ��� �� ������
	UInt32				components;				// ����� ���������
	UInt32				planes;					// ����� "������"
	
	// Image part
	UInt32				imgW;					// ������ ��������������� (����� ���������������) �����������
	UInt32				imgH;					// ������ ��������������� (����� ���������������) �����������
	UInt32				imgSW;					// �\� �������� �� �����������
	UInt32				imgSH;					// �\� �������� �� ���������

	UInt32				imgSPT;					//
	UInt32				imgSNT;					//
	UInt32				imgSIT;					//

	UInt32				imgSIN;					//
	UInt32				imgSJN;					//
	Int32				imgSTOLD;				//
	Int32				imgSTOCD;				//

	UInt32				imgCIN;					//
	UInt32				imgCJN;					//
	UInt32				imgCLM;					//
	UInt32				imgCCM;					//
	Int32				imgCTOLD;				//
	Int32				imgCTOCD;				//

	UInt32				imgCPT;					//
	UInt32				imgCNT;					//
	UInt32				imgCIT;					//

	UInt32				imgRotate;				//

	UInt16				*imgColY;				//
	UInt16				*imgColC;				//

	// EXIF part
	JPEG_ThumbType		thumb;					//

	// JPEG part
	UInt32				blockPresentField;		// ������� �����, ����������� ����������� ���� ��� ����� �������� (�����)
	StorageType			* storageP;				// ��������� �� storage, ��� ����� JPEG
	JPEG_InfoType		jpegInfo;				// ����� ���������� � JPEG�
	JPEG_MCUType		mcuInfo;				// ���������� � MCU
	JPEG_ScanType		scanInfo;				// ���������� � �����

	// temporary buffers
	UInt32 				unZigZagT[64];			// unzigzag �������
	Int16				cbCnv[256];				
	Int16				crCnv[256];

} JPEG_FormatType, * JPEG_FormatPtr;

typedef	Result IDCTProcType(Int32*,UInt8*,UInt8*,UInt32);
typedef	Result CopyProcType(JPEG_FormatType*,ImageType*,UInt8**,UInt32,UInt32,UInt32*);

///////////////////////////////////////////////////
//

typedef struct
{
	unsigned short		width;		// px
	unsigned short		height;		// px
	unsigned short		cNum;		// 3 (rgb color), 1 (grayscale) 

} JpegInfoType;


/////////////////////////////////////////////////////
//
#ifdef  __cplusplus
extern "C" {
#endif

unsigned long UTF_JPEG_IsJPEG( StorageType * srcP );
unsigned long UTF_JPEG_Open( void **jpgP, StorageType *srcP, EnvType * envP );
unsigned long UTF_JPEG_Close( void **jpgP, EnvType * envP );
unsigned long UTF_JPEG_GetInfo( void *jpgP, JpegInfoType *infP, EnvType * envP );
unsigned long UTF_JPEG_Load( void *jpgP, ImageType *imageP, unsigned long w, unsigned long h, unsigned long flags, unsigned short scaleFactor, EnvType * envP );
unsigned long UTF_JPEG_GetThumbnail( void * jpgH, ImageType * imageP, unsigned long w, unsigned long h, unsigned long flags, EnvType * envP );
unsigned long UTF_JPEG_StopDecoding();

#ifdef  __cplusplus
}
#endif

#endif //JPEG_DECODE_H

