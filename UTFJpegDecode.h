
#ifndef _UTF_JPEG_DECODE_H
#define _UTF_JPEG_DECODE_H


// FLAGS ////////////////////////////////////////////////////////////////////////////////////////
// таблица наличия блоков
#define jpegAPP0Present			0x0001	 // JFIF блок присутствует
#define jpegCOMPresent			0x0002	 // COM блок присутствует
#define jpegSOIPresent			0x0004	 // SOI блок присутствует
#define jpegEOIPresent			0x0008	 // EOI блок присутствует
#define jpegPreviewPresent		0x0010	 // в составе блока JFIF присутствует микрокартинка для превью
#define jpegSOF0Present			0x0020	 // SOF0 блок присутствует
#define jpegSOF2Present			0x0040	 // SOF0 блок присутствует
#define jpegDRIPresent			0x0080	 // DRI блок присутствует
#define jpegDQTPresent			0x0100	 // DQT блок присутствует
#define jpegDHTPresent			0x0200	 // DHT блок присутствует
#define jpegSOSPresent 			0x0400	 // SOS блок присутствует
#define jpegRSTxPresent 		0x0800	 // RSTx блок присутствует
#define jpegAPP1Present			0x1000	 // EXIF блок присутствует

// тип jpeg
#define jpegSingleScan			0x10000	 // последовательный, один скан
#define jpegMultipleScan		0x20000	 // последовательный, много сканов
#define jpegProgressive			0x40000	 // прогрессивный

#define SET_FLAG( jpg, flag )	(jpg->blockPresentField |= flag)
#define CHK_FLAG( jpg, flag )	((jpg->blockPresentField & flag) == flag)
#define CLR_FLAGS( jpg )		(jpg->blockPresentField = 0)
#define GET_FLAGS( jpg )		(jpg->blockPresentField)

// маркеры JPEG
#define jpegTEM    0xff01   // обычно вызывает ошибки декодирования, может быть проигнорировано
#define jpegSOF0   0xffc0   // Начало Кадра (Baseline JPEG)
#define jpegSOF1   0xffc1   // == SOF0
#define jpegSOF2   0xffc2   // прогрессивный
#define jpegSOF3   0xffc3   // 
#define jpegDHT    0xffc4   // Определение Таблицы Хаффмана
#define jpegSOF5   0xffc5   // 
#define jpegSOF6   0xffc6   // 
#define jpegSOF7   0xffc7   // 
#define jpegJPG    0xffc8   // неопределенный/зарезервированный (вызывает ошибки декодирования)
#define jpegSOF9   0xffc9   // для арифметического кодирования, обычно не поддерживается
#define jpegSOF10  0xffca   // обычно неподдерживаемый
#define jpegSOF11  0xffcb   // 
#define jpegDAC    0xffcc   // Определение Арифметической Таблицы, обычно не поддерживается
#define jpegSOF13  0xffcd   // 
#define jpegSOF14  0xffce   // 
#define jpegSOF15  0xffcf   // 
#define jpegRST0   0xffd0   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST1   0xffd1   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST2   0xffd2   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST3   0xffd3   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST4   0xffd4   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST5   0xffd5   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST6   0xffd6   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegRST7   0xffd7   // RSTn используются для синхронизации, может быть проигнорировано
#define jpegSOI    0xffd8   // Начало Изображения
#define jpegEOI    0xffd9   // Конец Изображения
#define jpegSOS    0xffda   // Начало Сканирования
#define jpegDQT    0xffdb   // Определение Таблицы Квантования
#define jpegDNL    0xffdc   // Определение Количество Строк, обычно неподдерживаемый, игнорировать
#define jpegDRI    0xffdd   // Определение Интервала Перезапуска
#define jpegAPP0   0xffe0   // JFIF маркеры сегментов приложения
#define jpegAPP1   0xffe1   // JFIF маркеры сегментов приложения
#define jpegAPP2   0xffe2   // JFIF маркеры сегментов приложения
#define jpegAPP3   0xffe3   // JFIF маркеры сегментов приложения
#define jpegAPP4   0xffe4   // JFIF маркеры сегментов приложения
#define jpegAPP5   0xffe5   // JFIF маркеры сегментов приложения
#define jpegAPP6   0xffe6   // JFIF маркеры сегментов приложения
#define jpegAPP7   0xffe7   // JFIF маркеры сегментов приложения
#define jpegAPP8   0xffe8   // JFIF маркеры сегментов приложения
#define jpegAPP9   0xffe9   // JFIF маркеры сегментов приложения
#define jpegAPP10  0xffea   // JFIF маркеры сегментов приложения
#define jpegAPP11  0xffeb   // JFIF маркеры сегментов приложения
#define jpegAPP12  0xffec   // JFIF маркеры сегментов приложения
#define jpegAPP13  0xffed   // JFIF маркеры сегментов приложения
#define jpegAPP14  0xffee   // JFIF маркеры сегментов приложения
#define jpegAPP15  0xffef   // игнорировать
#define jpegCOM    0xfffe   // Комментарий

////////////////////////////////////////////////////////////////
// 
#define resNull							0x00000000	// null
#define resOk							0x00000001	// ok
#define resFail							0x00000002	// fail
#define resBreak						0x00000003	// user break
#define resNoMem						0x00000030	// нет памяти

// storage 
#define resEOB							0x00000120	// достигнут конец блока
#define resBStOutOfRange				0x00000121	// запрашиваемый индекс вышел за пределы источника (файла)
#define resBStUncorrectPos				0x00000122	// некорректная позиция
#define resBStCannotOpen				0x00000123	// не удалось открыть источник (db/file)

// d.c.image
#define resInvalidDrawCtxForAttach		0x00000200	// невозможно приаттачить draw context к изображению
#define resUncompatDrawCtxForAttach		0x00000201	// несовместимость image и draw context
#define resInvalidImageCoord			0x00000202	// некорректные координаты изображения
#define resNoLockedImage				0x00000203	// изображение не было залочено
#define resNoRefreshCtxImage			0x00000204	// изображение не может обновить, связанный с ним контекст
#define resNoResizeImage				0x00000205	// изображение не может быть resize
#define resImageCrashed					0x00000206	// изображение разрушено! (нп,неудачным ресайзом) 
#define resImageCreated					0x00000207	// изображение уже создано
#define resImageNoCreated				0x00000208	// изображение не создано
#define resImageAttached				0x00000209	// изображение было приаттачено
#define resImageDettached				0x0000020a	// изображение было dettach
#define resImageInDynamic				0x0000020b	// изображение выделено в дин.памяти
#define resImageInStatic				0x0000020c	// изображение выделено в ст.памяти
#define resImageInDynamicOrStatic		0x0000020d	// изображение выделено в дин. или ст.памяти
#define resImageCannotRotate			0x0000020e	// изображение не может быть повернуто
#define resImageCannotResize			0x0000020f	// изображение не может быть resize
#define resInvalidLenSrcForResize		0x00000210	// изображение при resize содержит слишком мало точек
#define resCannotPackInvalidDB			0x00000211	// изображение не может быть запаковано; БД не может быть создана
#define resCannotUnpackInvalidDB		0x00000212	// изображение не может быть распаковано; БД не существует или не может быть открыта
#define resCannotPackExistDB			0x00000213	// изображение не может быть запаковано; БД уже существует 

// jpeg
#define resDoubledBlock					0x00001000	// предупреждение: блок уже встречался ранее
#define resIgnoredBlock					0x00001001	// предупреждение: блок игнорирован
#define resInvalidBlock					0x00001002	// фатальная ошибка: некорректный блок
#define resNoJPEG						0x00001003	// фатальная ошибка: не JPEG файл
#define resNoJFIF						0x00001004	// фатальная ошибка: не JFIF файл
#define resMayBeNoJFIF					0x00001005	// предупреждение: возможно не JFIF файл
#define resLossesData					0x00001006	// предупреждение: данные в файле не соответствуют меткам синхронизации
#define resUnknownVerJFIF				0x00001007	// фатальная ошибка: неизвестная версия JFIF файла
#define resUnsupportVerJFIF				0x00001008	// фатальная ошибка: неподдерживаемая версия JFIF файла
#define resUnsupportVerEXIF				0x00001009	// фатальная ошибка: неподдерживаемая версия EXIF файла
#define resUnsupportFormat				0x0000100a	// фатальная ошибка: неподдерживаемый формат
#define resUnsupportPrecision			0x0000100b	// фатальная ошибка: неподдерживаемый точность цвета (не 8бит/цвет)
#define resInvalidWidthHeight			0x0000100c	// фатальная ошибка: некорректный размер картинки (==0)
#define resUnsupportColorComp			0x0000100d	// фатальная ошибка: неподдерживаемое число компонент цвета (только 1-gray или 3-rgb)
#define resInvalidLenDRI				0x0000100e	// фатальная ошибка: неправильный размер блока DRI
#define resInvalidSizeQuantTable		0x0000100f	// фатальная ошибка: неправильный размер блока DQT
#define resInvalidNumQuantTable			0x00001010	// предупреждение: путаница с номерами таблиц дискретизации
#define resInvalidSizeHufTable			0x00001011	// фатальная ошибка: неправильный размер блока DHT
#define resInvalidNumHufTable			0x00001012	// предупреждение: путаница с номерами таблиц Хаффмана
#define resInvalidNumScanComp			0x00001013	// фатальная ошибка: неправильный число компонентов сканирования
#define resInvalidSizeSOSBlock			0x00001014	// фатальная ошибка: неправильный размер блока SOS
#define resUnadmitBlockId				0x00001015	// предупреждение: недопустимый id блока (например, 0x00000003)
#define res16bitQuantTable				0x00001016	// фатальная ошибка: с 16битными таблицами квантования пока не работаем
#define resInvalidRSTxMarker			0x00001017	// фатальная ошибка: некорректная последовательность маркеров перезапуска
#define resDehufCorrupt					0x00001018	// фатальная ошибка: не удалось декодировать по Хаффману блок DU (скорее всего битые данные)
#define resBlockFamilyCorrupt			0x00001019	// фатальная ошибка: блоки, необходимые для распаковки отсутствуют
#define resDoubledSOSBlock				0x0000101a	// фатальная ошибка: блок SOS уже встречался ранее
#define resJPEGCorrupt					0x0000101b	// фатальная ошибка: JPEG 'битый' (например, в конце файла нет маркера 0x000000d9)
#define resHufCodeNotFound				0x0000101c	// фатальная ошибка: при распаковке для очередной порции бит не был найден соответствующий код
#define resScalefactorNotSupported		0x0000101d	// фатальная ошибка: задан неверный параметр для ресэмплинга
#define resScaleUnusable				0x0000101e	// фатальная ошибка: задан слишком маленький (<1 к 8) параметр для ресэмплинга - нужно не масштабировать картинку, а распаковывать только DC
#define resWantOfBlocks					0x0000101f	// фатальная ошибка: не все сегменты присутствуют, распаковка прервана
#define resStartOfScan					0x00001020	// предупреждение: начало сканирования
#define resEndOfJpeg					0x00001021	// предупреждение: конец jpeg
#define resUnsupportedDiskretVal		0x00001022	// фатальная ошибка: неподдерживаемое значение дискрета > 2
#define resSpecSelectionError			0x00001023	// фатальная ошибка: в спектральном разделении
#define resSucesApproxError				0x00001024	// фатальная ошибка: в последовательном приближении
#define resPrgScanDataError				0x00001025	// фатальная ошибка: в данных прогрессивного скана
#define resPrgDUEOB						0x00001026	// предупреждение: найден end-of-band
#define resPrgUncorrectCompNum			0x00001027	// предупреждение: некорректное число компонент в прогрессивном скане
#define resExifCorrupt					0x00001028	// фатальная ошибка: некорректный EXIF block
#define resNoExif						0x00001029	// фатальная ошибка: не EXIF block
#define resExifUnsupportByteAlign		0x0000102a	// фатальная ошибка: неподдерживаемый формат следования байтов
#define resHufTooManyCodes				0x0000102b	// фатальная ошибка: при кодировании превышен лимит кодов
#define resHufMissingCodeSize			0x0000102c	// фатальная ошибка: при кодировании 
#define resHufDuplicateVal				0x0000102d	// фатальная ошибка: при кодировании 
#define resHufMissingValue				0x0000102e	// фатальная ошибка: при кодировании 
#define resHufInvalidRange				0x0000102f	// фатальная ошибка: при кодировании 
#define resHufDuplicateCode				0x00001030	// фатальная ошибка: при кодировании 

#ifndef LOPART 

#define LOPART(x)	((UInt8)((UInt8)(x) & 0x0f)) 
#define HIPART(x)	((UInt8)((UInt8)(x) >> 0x04)) 

#endif

#define jpegSegmentSize		0xfff0					// размер сегмента jpeg (предполагаемый), до 65Кб

#define jpegReserveSize		0x1000					// размер запаса (читается на jpegReserveSize байт больше)

#define jpegAPP0Sign		0x4a464946				// JFIF
#define jpegExifSign		0x45786966				// Exif
#define jpegEXIFSign		0x45584946				// EXIF
#define jpegAPP0SignXX		0x4a465858				// JFXX

#define jpegMaxBlockSize	0xffff					// максимальной рзмер блока
#define jpegBlockSign		0xff					// сигнатура начала блока
#define jpegMaxTableNum		4//3						// сейчас поддерживается только три таблицы квантования Y,Cr,Cb, а вообще может быть четыре - CMYK
#define jpegMaxHufTableNum	4//3						// сейчас поддерживается только 4 таблицы кодов Huffman
#define jpegMaxDiscretVal	10						// максимальное значение дискрет-ии для 4:4:1 -> 4 + 4 + 1 + запас (обычно 4:1:1, 1:1:1)
#define jpegMaxDUinMCU		4						// макс. число du в каждом mcu
#define jpegMaxComponentNum 4						// максимальное число компонентов
#define jpegMinHVSize		16						// минимальный размер картинки, с которым умеем работать

#define jpegDUHVSize		8						// размер блока DU по каждой стороне
#define jpegMCUMaxHSize		2 * jpegDUHVSize		// размер блока MCU по горизонтали в пикселах
#define jpegMCUMaxVSize		2 * jpegDUHVSize		// размер блока MCU по вертикали в пикселах
#define jpegMCUMaxSize		jpegMCUMaxHSize * jpegMCUMaxVSize

// id компонент изображения (CMYK мы не поддерживаем)
#define compY	0			// яркость
#define compCb	1			// цветоразность для синего
#define compCr	2			// цветоразность для красного
#define compCc	1			// cyan
#define compCm	2			// magenta
#define compCy	3			// yellow

// id компонент изображения (JFIF стандарт)
#define compYjf		1			// яркость
#define compCbjf	2			// цветоразность для синего
#define compCrjf	3			// цветоразность для красного
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
	UInt32				dhtInfo;			// информация о таблицах
	UInt32				dhtBits[16];		// только 16 и никак иначе
	UInt32				dhtCodes[256];		// больше 256 не может быть по определению

} JPEG_HufTableType;

typedef struct JPEG_HufTreeType
{
	UInt16				hufCode[256];				// больше 256 не может быть по определению
	UInt16				hufData[256];				// больше 256 не может быть по определению
	UInt16				hufSize[256];				// больше 256 не может быть по определению
	UInt32				hufLen;						// длины серий

} JPEG_HufTreeType;

typedef struct JPEG_QuantTableType			// не более 4-х таблиц 0..3
{
	UInt32				quantInfo;	
	UInt16				quantTable[65];		// пока умеем работать только с 8битными таблицами дискретизации

} JPEG_QuantTableType;

typedef struct JPEG_InfoType
{
// общая информация о JPEGe
	UInt32		width;							// реальная ширина
	UInt32 		height;							// реальная высота
	UInt32		compNum;						// число компонент
	UInt32		bitPerUnit;						// бит на каждый компонент

	UInt32		numMCU;							// общее число MCU
	UInt32		numMCUx;						// из них по горизонтали
	UInt32		numMCUy;						// из них по вертикали

	UInt32		widthM;							// ширина и высота картинки
	UInt32		heightM;						// выровненные по границе mcu

	UInt32		qntNum;							// общее число таблиц квантования
	UInt32		hufNum;							// общее число таблиц Хаффмана

	UInt32		resetInterval;					// интервал перезапуска, определяется маркером DRI	

} JPEG_InfoType;


typedef struct JPEG_MCUType
{
	UInt32		sizeX;							// размер по X распакованного блока MCU (реальный)
	UInt32		sizeY;							// размер по Y распакованного блока MCU (реальный)
	UInt32		sizeMCU;						// размер блока MCU (реальный)

	UInt32		vMax;							// макс. дискрет по вертикали
	UInt32		hMax;							// макс. дискрет по горизонтали
	UInt32		duInMcu;						// общее число du в mcu

	UInt32 		compNumInScan;					// число компонент в скане
	UInt32		compId[jpegMaxComponentNum];	// id компонент в скане
	UInt32		compMcuId[jpegMaxComponentNum];	// id компонент в mcu
	UInt32		seqMCU[jpegMaxComponentNum];	// размер блока MCU в блоках DU для каждого компонента
	
	UInt8		repVC[jpegMaxComponentNum];
	UInt8		repHC[jpegMaxComponentNum];
	UInt8		seqVC[jpegMaxComponentNum];
	UInt8		seqHC[jpegMaxComponentNum];
	
	Int32		DC[jpegMaxComponentNum];		// здесь будут накапливаться значения DC для каждого YCbCr компонента
												// 0-й элемент не используется просто для удобства, т.к. мин.номер компоненты - 1
	UInt32 		compPos[jpegMaxComponentNum];	// номера начальных позиций в массиве таблиц для каждого компонента
	UInt32		numHufTblDC[jpegMaxDiscretVal];	// номера таблиц Хаффмана для DC компонент [0] - для Y, [1] - для Cb, [2] - для Cr
	UInt32		numHufTblAC[jpegMaxDiscretVal];	// номера таблиц Хаффмана для AC компонент [0] - для Y, [1] - для Cb, [2] - для Cr
	UInt32		numQuantTbl[jpegMaxDiscretVal];	// номера таблиц квантования компонент [0] - для Y, [1] - для Cb, [2] - для Cr

} JPEG_MCUType;


typedef struct JPEG_ScanType
{
	UInt32				scanIndex;					// текущая позиция в скане
	UInt32				numScans;					// общее число сканов
	UInt32				scanSSS;					// Spectral Selection Start
	UInt32				scanSSE;					// Spectral Selection End
	UInt32				scanSAH;					// Sucessive Approximation High
	UInt32				scanSAL;					// Sucessive Approximation Low
	
// наборы таблиц квантования и Хаффмана (декодирования)
	JPEG_QuantTableType	quantTable[jpegMaxTableNum];			// всего три таблицы квантования
	JPEG_HufTreeType	hufDCTree[jpegMaxHufTableNum];			// дерево Хаффмана - в статике 3*256 байт
	JPEG_HufTreeType	hufACTree[jpegMaxHufTableNum];			// дерево Хаффмана - в статике 3*256 байт

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
	UInt32				len;					// длина структуры
	UInt32				width;					// ширина изображения
	UInt32				height;					// высота изображения
	UInt32				bitPerUnit;				// бит на пиксел
	UInt32				components;				// число компонент
	UInt32				planes;					// число "планов"
	
	// Image part
	UInt32				imgW;					// ширина результирующего (после масштабирования) изображения
	UInt32				imgH;					// высота результирующего (после масштабирования) изображения
	UInt32				imgSW;					// к\ф масштаба по горизонтали
	UInt32				imgSH;					// к\ф масштаба по вертикали

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
	UInt32				blockPresentField;		// битовые флаги, указывающие присутствие того или иного сегмента (блока)
	StorageType			* storageP;				// указатель на storage, где лежит JPEG
	JPEG_InfoType		jpegInfo;				// общая информация о JPEGе
	JPEG_MCUType		mcuInfo;				// информация о MCU
	JPEG_ScanType		scanInfo;				// информация о скане

	// temporary buffers
	UInt32 				unZigZagT[64];			// unzigzag таблица
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

