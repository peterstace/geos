// This file should document by example usage of the GEOS library.
// It could actually be a live discuss-by-example board for
// architectural design choices.
// 			--strk;
// 
// DEBUGGING TIPS:
//  use -D__USE_MALLOC at compile time for gcc 2.91, 2.95, 3.0 and 3.1
//  and GLIBCXX_FORCE_NEW or GLIBCPP_FORCE_NEW at run time with gcc 3.2.2+
//  to force libstdc++ avoid caching memory. This should remove some
//  obscure reports from memory checkers like valgrind.
// 
#include <stdio.h>
#include <io.h>
#include <geom.h>
#include <unload.h>

using namespace geos;

// This object will be used to construct our geometries.
// It might be bypassed by directly call geometry constructors,
// but that would be boring because you'd need to specify
// a PrecisionModel and a SRID everytime: those infos are
// cached inside a GeometryFactory object.
GeometryFactory *global_factory;

// This function will create a LinearRing
// geometry rapresenting a square with the given origin 
// and side 
LinearRing *
create_square_linearring(double xoffset, double yoffset, double side)
{
	// We will use a coordinate list to build the linearring
	CoordinateList *cl = new BasicCoordinateList(5);

	// Each coordinate in the list must be created,
	// passed to coordinate list setAt and then deleted.
	// Pretty boring uh ?
	Coordinate *c;
	c = new Coordinate(xoffset, yoffset);
	cl->setAt(*c ,0);
	delete c;
	c = new Coordinate(xoffset+side, yoffset);
	cl->setAt(*c ,1);
	delete c;
	c = new Coordinate(xoffset+side, yoffset+side);
	cl->setAt(*c ,2);
	delete c;
	c = new Coordinate(xoffset, yoffset+side);
	cl->setAt(*c ,3);
	delete c;
	c = new Coordinate(xoffset, yoffset);
	cl->setAt(*c ,4);
	delete c;

	// Now that we have a CoordinateList we can create 
	// the linearring.
	LinearRing *lr = (LinearRing*) global_factory->createLinearRing(cl);
	
	// We don't need our CoordinateList anymore, it has been 
	// copied inside the LinearRing object
	delete cl;

	return lr; // our LinearRing
}

// This function will create a Polygon
// geometry rapresenting a square with the given origin 
// and side and with a central hole 1/3 sided.
Polygon *
create_square_polygon(double xoffset, double yoffset, double side)
{
	// We need a LinearRing for the polygon shell 
	LinearRing *outer = create_square_linearring(xoffset,yoffset,side);

	// And another for the hole 
	LinearRing *inner = create_square_linearring(xoffset+(side/3),
			yoffset+(side/3),(side/3));
	
	// If we need to specify any hole, we do it using
	// a vector of Geometry pointers (I don't know why
	// not LinearRings)
	vector<Geometry *> *holes = new vector<Geometry *>;

	// We add the newly created geometry to the vector
	// of holes.
	holes->push_back(inner);

	// And finally we call the polygon constructor.
	// Both the outer LinearRing and the vector of holes
	// will be referenced by the resulting Polygon object,
	// thus we CANNOT delete them, neither the holes, nor
	// the vector containing their pointers, nor the outer
	// LinearRing. Everything will be deleted at Polygon
	// deletion time (this is inconsistent with LinearRing
	// behaviour... what should we do?).
	Polygon *poly = global_factory->createPolygon(outer, holes);

	return poly;
}

// This function will create a GeoemtryCollection
// containing the two given Geometries.
// Note that given Geometries will be referenced
// by returned object, thus deleted at its destruction
// time.
//
GeometryCollection *
create_simple_collection(Geometry *g1, Geometry *g2)
{
	// We need to construct a <Geometry *> vector
	// to use as argument to the factory function
	vector<Geometry *> *collection = new vector<Geometry *>;

	// Now, we need to make copies of the given args
	// we do it using copy constructor.
	collection->push_back(g1);
	collection->push_back(g2);

	GeometryCollection *ret =
		global_factory->createGeometryCollection(collection);


	// We HAVE to delete the vectore used to store
	// goemetry pointers, but created object will
	// delete pointed geometries, weird uh?
	delete collection;

	return ret;
}


// Start reading here
void do_all()
{
	int numgeoms = 3;
	Geometry *geoms[numgeoms];

	// Initialize global factory with default PrecisionModel
	// and SRID.
	global_factory = new GeometryFactory();

	// Read function bodies to see the magic behind them
	geoms[0] = create_square_linearring(0,0,100);
	geoms[1] = create_square_polygon(0,200,300);

	// here we write this bad-looking code to copy
	// geometries before putting them in a collection
	// object, since it will take responsability about
	// passed arguments life.
	geoms[2] = create_simple_collection(
			new LinearRing(*((LinearRing *)geoms[0])),
			new Polygon(*((Polygon *)geoms[1])));

	// WKT-print created geometries
	WKTWriter *wkt = new WKTWriter();
	for (int i=0; i<numgeoms; i++) {
		cout<<wkt->write(geoms[i])<<endl;
	}
	delete wkt;

	// Delete created geometries
	for (int i=0; i<numgeoms; i++) {
		delete geoms[i];
	}

	delete global_factory;
}

main()
{
	try
	{
		do_all();
	}
	// All exception thrown by GEOS are subclasses of this
	// one, so this is a catch-all 
	catch (GEOSException *exc)
	{
		cerr <<"Generic exception: "<<exc->toString()<<"\n";
		exit(1);
	}
	// and this is a catch-all non standard ;)
	catch (...)
	{
		cerr <<"unknown exception trown!\n";
		exit(1);
	}

	// This is not really needed but to make
	// memory checker like valgrind quiet
	// about static heap-allocated data.
	Unload::Release();
}
