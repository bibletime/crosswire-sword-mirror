#include <utilxml.h>
#include <iostream>

using namespace sword;
using namespace std;

int main(int argc, char **argv) {

	const char *xml = "<verse osisID=\"John.1.1\" type=\'test type\' yeah = \"stuff\" />";
	cout << ((argc > 1) ? argv[1]: xml) << "\n";

	XMLTag x((argc > 1) ? argv[1] : xml);

	cout << x.toString() << "\n";
	x.setAttribute("addedAttribute", "with a \" quote");
	cout << x.toString() << "\n";
	cout << "Tag name: [" << x.getName() << "]\n";
	StringList attributes = x.getAttributeNames();
	for (StringList::iterator it = attributes.begin(); it != attributes.end(); it++) {
		const char *name = it->c_str();
		cout << " - attribute: [" << name << "] = [";
		cout << x.getAttribute(name) << "]\n";
		int count = x.getAttributePartCount(name, ' ');
		cout << "\t" << count << " parts:\n";
		int i = (count > 1) ? 0 : -1;		// -1 for whole value cuz it's faster, but does the same thing as 0
		do {
			cout << "\t" << x.getAttribute(name, i, ' ') << "\n";
			if (i < 0) i = 0;	// to handle our -1 condition
		} while (++i < count);
	}
	cout << " isEmpty: " << x.isEmpty() << "\n";
	cout << " isEndTag: " << x.isEndTag() << "\n";
	cout << "\n";

	
	if (argc < 2) {  // only run if we're defaulted
		const char *testParts="ABC D EF GHIJ";
		cout << "Setting attribute 'multiPart' to: '" << testParts << "'\n";
		x.setAttribute("multiPart", testParts);
		cout << x << "\n";
		cout << "Setting part 2 to 'MMM'\n";
		x.setAttribute("multiPart", "MMM", 2, ' ');
		cout << x << "\n";
		cout << "Removing part 1\n";
		x.setAttribute("multiPart", 0, 1, ' ');
		cout << x << "\n";
		cout << "Removing part 2\n";
		x.setAttribute("multiPart", 0, 2, ' ');
		cout << x << "\n";
	}
	
}
