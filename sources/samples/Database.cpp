/*****************************************************************

        O P E N      M E D I A     T O O L K I T              V2.5    
 
        Copyright Yves Schmid 1996-2003
 
        See www.garagecube.com for more informations about this library.
        
        Author(s): Yves Schmid
 
        OMT is provided under LGPL:
 
          This library is free software; you can redistribute it and/or
          modify it under the terms of the GNU Lesser General Public
          License as published by the Free Software Foundation; either
          version 2.1 of the License, or (at your option) any later version.

          This library is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
          Lesser General Public License for more details.

          You should have received a copy of the GNU Lesser General Public
          License along with this library; if not, write to the Free Software
          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

          The full text of the license can be found in lgpl.txt          

******************************************************************/

// Database and streaming example

#include "OMediaMemStream.h"
#include "OMediaClassStreamer.h"
#include "OMediaDBObject.h"
#include "OMediaDataBase.h"

#include <stdio.h>

//--------------------------------------------------------
// This an example of how to create a streameable object

class StreamObject : public OMediaClassStreamer
{

public:

	long	x,y,z;

	// Lecture de l'objet

	virtual void read_class(OMediaStreamOperators &stream)
	{
		stream>>x;
		stream>>y;
		stream>>z;
	}

	// Ecriture de l'objet

	virtual void write_class(OMediaStreamOperators &stream)
	{
		stream<<x;
		stream<<y;
		stream<<z;
	}

};

//--------------------------------------------------------
// And now an example of a full database object

class DataBaseObject : public OMediaDBObject
{
public:

	float	x,y,z;

	// The db_type is used to identify the object class

	enum {db_type = 'Test'};

	DataBaseObject()
	{
		x = y = z = 0;

		// By default objects are not compressed. If you want compression
		// you need to set a compression_level. The following line set the
		// default compression level.
		compression_level = omclc_DefaultCompression;	
	}

	virtual ~DataBaseObject()
	{
		db_update();	// You must call this method in your destructor each time
						// you declare read/write_class methods in your class.
						// This call update the object if required.
	}


	// Streaming object

	virtual void read_class(OMediaStreamOperators &stream)
	{
		OMediaDBObject::read_class(stream);	// Prepare database

		stream>>x;	// Read your data
		stream>>y;
		stream>>z;
	}

	virtual void write_class(OMediaStreamOperators &stream)
	{
		OMediaDBObject::write_class(stream);	// Prepare database

		stream<<x;	// Write your data
		stream<<y;
		stream<<z;
	}


	// Object builder. This static method is used by the database to build
	// new object of this type. It is associated with the db_type when you
	// register the class.
	static OMediaDBObject *db_builder(void) {return new DataBaseObject;}

        // A dynamic get_type 
	virtual unsigned long db_get_type(void) const
        {
            return db_type;
        }


	// Following method should returns the memory used by this object. It is
	// used by the database caching to know when the cache is full and some
	// objects should be removed from memory.
	unsigned long get_approximate_size(void) {return sizeof(*this);}
};



//--------------------------------------------------------


int main(void)
{
	OMediaDataBase			*database;
	OMediaFormattedStream	*fmtstream;
	OMediaMemStream			*stream;
	string					test = "This is a test";
	StreamObject			str_object;

	//--------------------------------------------------------
	// Simple stream

	str_object.x = 0;	str_object.y = 1;	str_object.z = 2;

	printf("Simple stream test.\n");

	stream = new OMediaMemStream;
	*stream<<test;
	*stream<<str_object;

	test = "";
	str_object.x = -1;	str_object.y = -1;	str_object.z = -1;

	stream->setposition(0);			// Read back to see if it works!
	*stream>>test;
	*stream>>str_object;

	delete stream;

	//--------------------------------------------------------

	// Formatted stream

	printf("Formatted stream test.\n");


	// Open the base stream. Once open it is not directly used anymore.
	stream = new OMediaMemStream;

	// Create a formatted stream. The formatted stream will format the base
	// stream.
	fmtstream = new OMediaFormattedStream(stream);

	// Open a chunk
	fmtstream->open_chunk('Test',0);
	*fmtstream<<str_object;				// Write a simple object

	// Open another chunk (open_chunk closes the last chunk before opening the new one)
	fmtstream->open_chunk('Test',1);
	str_object.x = -1;	str_object.y = -1;	str_object.z = -1;
	*fmtstream<<str_object;				// Write second object

	// Reouvre le premier chunk
	fmtstream->open_chunk('Test',0);
	*fmtstream>>str_object;				// Read back object 0

	delete fmtstream;
	delete stream;

	//--------------------------------------------------------
	// Database

	printf("Database test.\n");

	// Register the class. So databases will recognize it by its db_type

	omd_REGISTERCLASS(DataBaseObject);

	// Open a mem stream for testing purpose
	stream = new OMediaMemStream;

	// Create database

	database = new OMediaDataBase(stream);

	// I ask for objet 0. If it does not exist, it is automatically created

	DataBaseObject	*obj;
	
	obj = (DataBaseObject*) database->get_object(DataBaseObject::db_type,0);

	obj->x = 1;	obj->y = 2;	obj->z = 3;
	obj->set_modified();	// Object is dirty. If I don't set this flag, object will
							// be updated
	obj->db_unlock();

	// Close the database

	delete database;

	//... and reopen it

	database = new OMediaDataBase(stream);

	// Get back my saved object

	obj = (DataBaseObject*) database->get_object(DataBaseObject::db_type,0);

	delete database;
	delete stream;

	// That's all folks!
        
        return 0;
}

