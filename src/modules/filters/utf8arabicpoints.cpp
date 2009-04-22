/******************************************************************************
 *
 * UTF8ArabicPoints - SWFilter descendant to remove UTF-8 Arabic vowel points
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <utf8arabicpoints.h>

SWORD_NAMESPACE_START

const char oName[] = "Arabic Vowel Points";
const char oTip[] = "Toggles Arabic Vowel Points";

const SWBuf choices[3] = {"On", "Off", ""};
const StringList oValues(&choices[0], &choices[2]);

UTF8ArabicPoints::UTF8ArabicPoints() : SWOptionFilter(oName, oTip, &oValues) {
	setOptionValue("On");
}

UTF8ArabicPoints::~UTF8ArabicPoints(){};


char UTF8ArabicPoints::processText(SWBuf &text, const SWKey *key, const SWModule *module) {
	if (!option) {
		//The UTF-8 range 0xFC 0xE5 to 0xFC 0x63 consist of Arabic vowel marks so block those out.
		// Also ranges 0xFE70 til OxFE7F and 0x064b-0x0655
		SWBuf orig = text;
		const unsigned char* from = (unsigned char*)orig.c_str();
		for (text = ""; *from; from++) {
			if (((*from == 0xFC) && (*(from + 1) >= 0xE5 && *(from + 1) <= 0x63)) || ((*from == 0xFE) && (*(from + 1) >= 0x70 && *(from + 1) <= 0x7F)) || ((*from == 0x06) && (*(from + 1) >= 0x4B && *(from + 1) <= 0x55))){
				from++;
			}
			else {
     			        text += *from;
                        }
		}
     	}
	return 0;
}

SWORD_NAMESPACE_END
