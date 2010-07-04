///////////////////////////////////////////////////////////////////////////
// Author: Martial TOLA
// Year: 2010
// CNRS-Lyon, LIRIS UMR 5205
/////////////////////////////////////////////////////////////////////////// 
#include "scene.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4267)
#include <CGAL/IO/Polyhedron_iostream.h>
#pragma warning(pop)
#else
#include <CGAL/IO/Polyhedron_iostream.h>
#endif

#include <QApplication>
#include <QMessageBox>

#include <fstream>

#include "viewer.hxx"

Scene::Scene()
{
	m_current_polyhedron = -1;
	m_loadType = Normal; 
}

Scene::~Scene()
{
	m_polyhedrons.clear();
}

int Scene::add_mesh(QString filename, int loadType, typeFuncOpenSave f, Viewer* viewer)
{
	int res = 0;

	m_loadType = loadType;

	PolyhedronPtr polyhedron_ptr(new Polyhedron());

	if (filename == EMPTY_MESH)
	{
		// nothing
	}
	else if (filename == INTERNAL_MESH)
	{
		Point3d p1( -0.5, -0.5, -0.5);
		Point3d q1( 0.5, -0.5, -0.5);
		Point3d r1( 0.0, -0.5, 0.5);
		Point3d s1( 0.0, 0.5, 0.0);

		Halfedge_handle h = polyhedron_ptr->make_tetrahedron(p1, q1, r1, s1);
		if (!polyhedron_ptr->is_tetrahedron(h))
			res = -3;
	}
	else
	{
		if (f != NULL)
			res = f(polyhedron_ptr, filename, viewer);
		else
		{
			QString ext = QFileInfo(filename).suffix();

			if (ext == "off")
				res = polyhedron_ptr->load_mesh_off(filename.toStdString());
			else if (ext == "obj")
				res = polyhedron_ptr->load_mesh_obj(filename.toStdString());
			else if (ext == "smf")
				res = polyhedron_ptr->load_mesh_smf(filename.toStdString());
			else if (ext == "ply")
				res = polyhedron_ptr->load_mesh_ply(filename.toStdString());
			else
				res = 1;
		}
	}

	if (!res)
	{
		curFile = filename;

		if (!polyhedron_ptr->empty())
		{
			polyhedron_ptr->compute_bounding_box();

			polyhedron_ptr->compute_normals();
			polyhedron_ptr->compute_type();

			(void)polyhedron_ptr->calc_nb_components();
			(void)polyhedron_ptr->calc_nb_boundaries();
		}

		add_polyhedron(polyhedron_ptr);
		set_current_polyhedron(get_nb_polyhedrons()-1);

		// mode Space
		if (m_loadType==Space && (m_polyhedrons.size() > (unsigned)viewer->get_nb_frames()))
		{
			if ((unsigned)viewer->get_nb_frames()==0)
				viewer->addFrame();
			viewer->addFrame();

			unsigned short i = (unsigned)(viewer->get_nb_frames()-1);
			viewer->setManipulatedFrame(viewer->frame(i));
			viewer->setSelectedFrameNumber(i);

			int m = i%2;

			if (m)
				viewer->frame(i)->setPosition(qglviewer::Vec(0.f, 1.1 * ceil(i/2.), 0.f));
			else
				viewer->frame(i)->setPosition(qglviewer::Vec(0.f, -1.1 * ceil(i/2.), 0.f));
		}
		// mode Space
	}

	return res;
}

int Scene::load_file(PolyhedronPtr polyhedron_ptr, QString filename)
{
	QApplication::setOverrideCursor(QCursor(::Qt::WaitCursor));

	QFileInfo fileinfo(filename);
	std::ifstream in(filename.toUtf8());
	
	if (!in || !fileinfo.isFile() || ! fileinfo.isReadable())
	{
		QApplication::restoreOverrideCursor();
		return -1;
	}

	in >> *polyhedron_ptr;

	if (!in)
	{
		QApplication::restoreOverrideCursor();

		return -1;
	}

	QApplication::restoreOverrideCursor();

	return 0;
}

int Scene::save_file(QString filename, typeFuncOpenSave f, Viewer* viewer)
{
	QApplication::setOverrideCursor(QCursor(::Qt::WaitCursor));

	int res = 0;
	QFileInfo fileinfo(filename);
	std::ofstream out(filename.toUtf8());

	if (!out || !fileinfo.isFile() || ! fileinfo.isWritable())
	{
		QApplication::restoreOverrideCursor();
		return -1;
	}

	if (f != NULL)
		res = f(get_polyhedron(), filename, viewer);
	else
	{
		out << *get_polyhedron();

		if (!out)
		{
			QApplication::restoreOverrideCursor();
			return -1;
		}
	}

	QApplication::restoreOverrideCursor();

	return res;
}