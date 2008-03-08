#include <vector>
#include <algorithm>
#include <cmath>

#include <cvd/vector_image_ref.h>
#include <cvd/image.h>

#include <TooN/TooN.h>

#include <tag/stdpp.h>
#include <tag/fn.h>

#include <gvars3/instances.h>

#include "offsets.h"

using namespace std;
using namespace CVD;
using namespace TooN;
using namespace tag;
using namespace GVars3;




///Actual x,y offset of the offset numbers in the different available orientations.
///@ingroup gTree
vector<vector<ImageRef> > offsets;
///The number of possible offsets. Equivalent to <code>offsets[x].size()</code>
///@ingroup gTree
int num_offsets;
///Bounding box for offsets in all orientations. This is therefore a bounding box for the detector.
///@ingroup gTree
pair<ImageRef, ImageRef> offsets_bbox;




///Rotate a vector<ImageRef> by a given angle, with an optional reflection.
///@param offsets Offsets to rotate.
///@param angle Angle to rotate by.
///@param r Whether to reflect.
///@return The rotated offsets.
///@ingroup gTree
vector<ImageRef> transform_offsets(const vector<ImageRef>& offsets, int angle, bool r)
{
	double a = angle * M_PI / 2;	

	double R_[] = { cos(a), sin(a), -sin(a) , cos(a) };
	double F_[] = { 1, 0, 0, r?-1:1};

	Matrix<2> R(R_), F(F_);
	Matrix<2> T = R*F;

	vector<ImageRef> ret;

	for(unsigned int i=0; i < offsets.size(); i++)
	{
		Vector<2> v = vec(offsets[i]);
		ret.push_back(ir_rounded(T * v));
	}
	
	return ret;
}


///Pretty print some offsets to stdout.
///@param offsets List of offsets to pretty-print.
///@ingroup gUtility
void draw_offset_list(const vector<ImageRef>& offsets)
{

	cout << "Allowed offsets: " << offsets.size() << endl;

	ImageRef min, max;
	min.x = *min_element(member_iterator(offsets.begin(), &ImageRef::x), member_iterator(offsets.end(), &ImageRef::x));
	max.x = *max_element(member_iterator(offsets.begin(), &ImageRef::x), member_iterator(offsets.end(), &ImageRef::x));
	min.y = *min_element(member_iterator(offsets.begin(), &ImageRef::y), member_iterator(offsets.end(), &ImageRef::y));
	max.y = *max_element(member_iterator(offsets.begin(), &ImageRef::y), member_iterator(offsets.end(), &ImageRef::y));

	cout << print << min <<max << endl;

	Image<int> o(max-min+ImageRef(1,1), -1);
	for(unsigned int i=0; i <offsets.size(); i++)
		o[offsets[i] -min] = i;

	for(int y=0; y < o.size().y; y++)
	{
		for(int x=0; x < o.size().x; x++)
			cout << "+------";
		cout << "+"<< endl;

		for(int x=0; x < o.size().x; x++)
			cout << "|      ";
		cout << "|"<< endl;


		for(int x=0; x < o.size().x; x++)
		{
			if(o[y][x] >= 0)
				cout << "|  " << setw(2) << o[y][x] << "  ";
			else if(ImageRef(x, y) == o.size() / 2)
				cout << "|   " << "#" << "  ";
			else 
				cout << "|      ";
		}
		cout <<  "|" << endl;

		for(int x=0; x < o.size().x; x++)
			cout << "|      ";
		cout << "|"<< endl;
	}

	for(int x=0; x < o.size().x; x++)
		cout << "+------";
	cout << "+"<< endl;

	cout << endl;

}


void create_offsets()
{
	//Pixel offsets are represented as integer indices in to an array of
	//ImageRefs. That means that by choosing the array, the tree can be
	//rotated and/or reflected. Here, an annulus of possible offsets is 
	//created and rotated by all multiples of 90 degrees, and then reflected.
	//This gives a total of 8.
	offsets.resize(8);
	{	
		double min_r = GV3::get<double>("offsets.min_radius");
		double max_r = GV3::get<double>("offsets.max_radius");

		ImageRef max((int)ceil(max_r+1), (int)ceil(max_r+1));
		ImageRef min = -max, p = min;

		cout << "Offsets: ";

		do
		{
			double d = vec(p) * vec(p);

			if(d >= min_r*min_r && d <= max_r * max_r)
			{
				offsets[0].push_back(p);
				cout << offsets[0].back() << " ";
			}
		}
		while(p.next(min, max));

		cout << endl;

		offsets_bbox = make_pair(min, max);
	}
	offsets[1] = transform_offsets(offsets[0], 1, 0);
	offsets[2] = transform_offsets(offsets[0], 2, 0);
	offsets[3] = transform_offsets(offsets[0], 3, 0);
	offsets[4] = transform_offsets(offsets[0], 0, 1);
	offsets[5] = transform_offsets(offsets[0], 1, 1);
	offsets[6] = transform_offsets(offsets[0], 2, 1);
	offsets[7] = transform_offsets(offsets[0], 3, 1);
	num_offsets=offsets[0].size();
}



void draw_offsets()
{
	//Print the offsets out.	
	for(unsigned int i=0; i < 8; i++)
	{
		cout << "Offsets " << i << endl;
		draw_offset_list(offsets[i]);	
		cout << endl;
	}
}