#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>


#define NULLPOINT3 Point3(999999999,999999999,999999999)  //���������ֵ�ĵ�Ϊ�յ�
#define ACCURACYLIMIT 1e-13

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point3;
typedef K::Point_2 Point2;
typedef K::Plane_3 Plane3;
typedef K::Triangle_3 Triangle3;
typedef K::Intersect_3 Intersect_3;
typedef K::Segment_3 Segment3;
typedef CGAL::Delaunay_triangulation_2<K> Triangulation;

typedef struct {

	int *FList;
	int FN;
	double *PList;
	int PN;
}*VFSTRUCT;

extern "C" _declspec(dllexport)  double _stdcall Cut(double *p, int *f, int pn, int fn, double *m, int *f1, int *f2, int *sides, int* outpn)
{

	int side1 = 0, side2 = 0;   //���и������� ����*3
	std::vector<Point3> PointList;  //����ĵ�����
	std::vector <std::array<int, 3>> FaceList; //�����������

	//�����и���
	Point3 m1(m[0], m[1], m[2]);
	Point3 m2(m[3], m[4], m[5]);
	Point3 m3(m[6], m[7], m[8]);
	Plane3 plane3 = Plane3(m1, m2, m3);

	std::vector<Point3> NewPointsList3(pn, NULLPOINT3); //�����и����ϵĵ������ʼ��Ϊpn��

	//��������PointList��ֵ
	for (int i = 0; i < pn; i++)
	{
		PointList.push_back(Point3(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]));
	}
	//��������FaceList��ֵ
	for (int i = 0; i < fn; i++)
	{
		FaceList.push_back({ f[i * 3], f[i * 3 + 1], f[i * 3 + 2] });
	}


	//��ÿ������б���
	for (int i = 0; i < fn; i++)
	{
		std::vector<Point3> pt3n, pt3p, pt3c;  //���ֵ
		std::vector<int> fcn, fcp, fcc;   //����PointList�е��±�

		//��ÿ�����ϵ���������з���
		for (int j = 0; j < 3; j++)
		{
			switch (CGAL::orientation(m1, m2, m3, PointList[FaceList[i][j]]))
			{
			case -1:		
				pt3n.push_back(PointList[FaceList[i][j]]);
				fcn.push_back(FaceList[i][j]);
				break;
			case 1:
				pt3p.push_back(PointList[FaceList[i][j]]);
				fcp.push_back(FaceList[i][j]);
				break;
			case 0:
				pt3c.push_back(PointList[FaceList[i][j]]);
				fcc.push_back(FaceList[i][j]);
				NewPointsList3[FaceList[i][j]] = PointList[FaceList[i][j]];
				break;
			}
		}

		/****************�����ǶԸ�������������*************/

		//���������㶼���и����ϣ��������غϣ�ʱ
		if (pt3c.size() == 3)
		{
			f1[side1] = FaceList[i][0];
			side1++;
			f1[side1] = FaceList[i][1];
			side1++;
			f1[side1] = FaceList[i][2];
			side1++;

			f2[side2] = FaceList[i][0];
			side2++;
			f2[side2] = FaceList[i][1];
			side2++;
			f2[side2] = FaceList[i][2];
			side2++;

		}

		//���������и����n��
		if ((pt3n.size() >= 2 && pt3p.size() == 0) || (pt3n.size() == 1 && pt3c.size() == 2))
		{
			f1[side1] = FaceList[i][0];
			side1++;
			f1[side1] = FaceList[i][1];
			side1++;
			f1[side1] = FaceList[i][2];
			side1++;

		}

		//���������и����p��
		if ((pt3n.size() == 0 && pt3p.size() >= 2) || (pt3p.size() == 1 && pt3c.size() == 2))
		{
			f2[side2] = FaceList[i][0];
			side2++;
			f2[side2] = FaceList[i][1];
			side2++;
			f2[side2] = FaceList[i][2];
			side2++;
		}

		//����һ�������и����غϣ����������ֱ���n����p��
		if (pt3n.size() == 1 && pt3p.size() == 1 && pt3c.size() == 1)
		{
			Triangle3 t = Triangle3(PointList[FaceList[i][0]], PointList[FaceList[i][1]], PointList[FaceList[i][2]]);
			CGAL::cpp11::result_of<Intersect_3(Plane3, Triangle3)>::type result = intersection(plane3, t);
			Segment3* s3 = boost::get<Segment3>(&*result);

			if (pt3c[0] != (Point3)(*s3)[0])
			{
				PointList.push_back((Point3)(*s3)[0]);
				p[PointList.size() * 3 - 3] = ((Point3)(*s3)[0])[0];
				p[PointList.size() * 3 - 2] = ((Point3)(*s3)[0])[1];
				p[PointList.size() * 3 - 1] = ((Point3)(*s3)[0])[2];

				NewPointsList3.push_back((Point3)(*s3)[0]);
			}
			else
			{
				PointList.push_back((Point3)(*s3)[1]);
				p[PointList.size() * 3 - 3] = ((Point3)(*s3)[1])[0];
				p[PointList.size() * 3 - 2] = ((Point3)(*s3)[1])[1];
				p[PointList.size() * 3 - 1] = ((Point3)(*s3)[1])[2];

				NewPointsList3.push_back((Point3)(*s3)[1]);
			}
			f1[side1] = fcn[0];
			side1++;
			f1[side1] = fcc[0];
			side1++;
			f1[side1] = PointList.size() - 1;
			side1++;

			f2[side2] = fcp[0];
			side2++;
			f2[side2] = fcc[0];
			side2++;
			f2[side2] = PointList.size() - 1;
			side2++;
		}

		//��������n�棬һ������p��
		if (pt3n.size() == 2 && pt3p.size() == 1)
		{
			Triangle3 t = Triangle3(PointList[FaceList[i][0]], PointList[FaceList[i][1]], PointList[FaceList[i][2]]);
			CGAL::cpp11::result_of<Intersect_3(Plane3, Triangle3)>::type result = intersection(plane3, t);
			Segment3* s3 = boost::get<Segment3>(&*result);

			//�������������
			PointList.push_back((Point3)(*s3)[0]);
			PointList.push_back((Point3)(*s3)[1]);

			NewPointsList3.push_back((Point3)(*s3)[0]);
			NewPointsList3.push_back((Point3)(*s3)[1]);

			p[PointList.size() * 3 - 6] = ((Point3)(*s3)[0])[0];
			p[PointList.size() * 3 - 5] = ((Point3)(*s3)[0])[1];
			p[PointList.size() * 3 - 4] = ((Point3)(*s3)[0])[2];
			p[PointList.size() * 3 - 3] = ((Point3)(*s3)[1])[0];
			p[PointList.size() * 3 - 2] = ((Point3)(*s3)[1])[1];
			p[PointList.size() * 3 - 1] = ((Point3)(*s3)[1])[2];

			std::vector<Point2> tmpPoints2; //�����гɺ���4��������е�ӳ��Ϊ2D

			if (fabs(PointList[fcn[0]][0] - PointList[PointList.size() - 1][0])< ACCURACYLIMIT &&fabs(PointList[fcn[0]][0] - PointList[PointList.size() - 2][0])<ACCURACYLIMIT)   //���������ƽ����Y-Z��
			{
				tmpPoints2.push_back(Point2(PointList[fcn[0]][1], PointList[fcn[0]][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][1], PointList[PointList.size() - 1][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][1], PointList[PointList.size() - 2][2]));
				tmpPoints2.push_back(Point2(PointList[fcn[1]][1], PointList[fcn[1]][2]));
			}
			else if (fabs(PointList[fcn[0]][1] - PointList[PointList.size() - 1][1]) < ACCURACYLIMIT && fabs(PointList[fcn[0]][1] - PointList[PointList.size() - 2][1]) < ACCURACYLIMIT) //���������ƽ����X-Z��
			{
				tmpPoints2.push_back(Point2(PointList[fcn[0]][0], PointList[fcn[0]][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][2]));
				tmpPoints2.push_back(Point2(PointList[fcn[1]][0], PointList[fcn[1]][2]));
			}
			else        //���������ƽ����X-Y��򶼲�ƽ��
			{
				tmpPoints2.push_back(Point2(PointList[fcn[0]][0], PointList[fcn[0]][1]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][1]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][1]));
				tmpPoints2.push_back(Point2(PointList[fcn[1]][0], PointList[fcn[1]][1]));
			}

			Triangulation T;
			T.insert(tmpPoints2.begin(), tmpPoints2.end());
			Triangulation::Finite_faces_iterator Finite_face_iterator;

			for (Finite_face_iterator = T.finite_faces_begin(); Finite_face_iterator != T.finite_faces_end(); ++Finite_face_iterator)
			{
				Point2 p0 = Point2(T.triangle(Finite_face_iterator).vertex(0));
				Point2 p1 = Point2(T.triangle(Finite_face_iterator).vertex(1));
				Point2 p2 = Point2(T.triangle(Finite_face_iterator).vertex(2));

				std::vector<Point2>::iterator iter;
				int F[3];
				for (iter = tmpPoints2.begin(); iter != tmpPoints2.end(); iter++)
				{
					if (fabs((*iter)[0] - p0[0])< ACCURACYLIMIT && fabs((*iter)[1] - p0[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[0] = fcn[0];
							break;
						case 1:
							F[0] = PointList.size() - 1;
							break;
						case 2:
							F[0] = PointList.size() - 2;
							break;
						case 3:
							F[0] = fcn[1];
							break;
						}
					}
					 if (fabs((*iter)[0] - p1[0])< ACCURACYLIMIT && fabs((*iter)[1] - p1[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[1] = fcn[0];
							break;
						case 1:
							F[1] = PointList.size() - 1;
							break;
						case 2:
							F[1] = PointList.size() - 2;
							break;
						case 3:
							F[1] = fcn[1];
							break;
						}
					}
					 if (fabs((*iter)[0] - p2[0])< ACCURACYLIMIT && fabs((*iter)[1] - p2[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[2] = fcn[0];
							break;
						case 1:
							F[2] = PointList.size() - 1;
							break;
						case 2:
							F[2] = PointList.size() - 2;
							break;
						case 3:
							F[2] = fcn[1];
							break;
						}
					}
				}

				f1[side1] = F[0];
				side1++;
				f1[side1] = F[1];
				side1++;
				f1[side1] = F[2];
				side1++;
			}

			///////////////////

			std::vector<Point2> tmpPoints21;
			Triangulation T1;

			if (fabs(PointList[fcp[0]][0] - PointList[PointList.size() - 1][0]) < ACCURACYLIMIT && fabs(PointList[fcp[0]][0] - PointList[PointList.size() - 2][0]) < ACCURACYLIMIT)
			{
				tmpPoints21.push_back(Point2(PointList[fcp[0]][1], PointList[fcp[0]][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][1], PointList[PointList.size() - 1][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][1], PointList[PointList.size() - 2][2]));
			}
			else if (fabs(PointList[fcp[0]][1] - PointList[PointList.size() - 1][1])< ACCURACYLIMIT && fabs(PointList[fcp[0]][1] - PointList[PointList.size() - 2][1]) < ACCURACYLIMIT)
			{
				tmpPoints21.push_back(Point2(PointList[fcp[0]][0], PointList[fcp[0]][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][2]));
			}
			else
			{
				tmpPoints21.push_back(Point2(PointList[fcp[0]][0], PointList[fcp[0]][1]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][1]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][1]));
			}

			T1.insert(tmpPoints21.begin(), tmpPoints21.end());
			Triangulation::Finite_faces_iterator Finite_face_iterator1;


			for (Finite_face_iterator1 = T1.finite_faces_begin(); Finite_face_iterator1 != T1.finite_faces_end(); ++Finite_face_iterator1)
			{
				Point2 p0 = Point2(T1.triangle(Finite_face_iterator1).vertex(0));
				Point2 p1 = Point2(T1.triangle(Finite_face_iterator1).vertex(1));
				Point2 p2 = Point2(T1.triangle(Finite_face_iterator1).vertex(2));

				std::vector<Point2>::iterator iter;
				int F[3];
				for (iter = tmpPoints21.begin(); iter != tmpPoints21.end(); iter++)
				{
					if (fabs((*iter)[0] - p0[0])< ACCURACYLIMIT && fabs((*iter)[1] - p0[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[0] = fcp[0];
							break;
						case 1:
							F[0] = PointList.size() - 1;
							break;
						case 2:
							F[0] = PointList.size() - 2;
							break;

						}
					}
					if (fabs((*iter)[0] - p1[0])< ACCURACYLIMIT && fabs((*iter)[1] - p1[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[1] = fcp[0];
							break;
						case 1:
							F[1] = PointList.size() - 1;
							break;
						case 2:
							F[1] = PointList.size() - 2;
							break;
						}
					}
					if (fabs((*iter)[0] - p2[0])< ACCURACYLIMIT && fabs((*iter)[1] - p2[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[2] = fcp[0];
							break;
						case 1:
							F[2] = PointList.size() - 1;
							break;
						case 2:
							F[2] = PointList.size() - 2;
							break;

						}
					}
				}

				f2[side2] = F[0];
				side2++;
				f2[side2] = F[1];
				side2++;
				f2[side2] = F[2];
				side2++;
			}

		}

		//��������p�棬һ������n��
		if (pt3n.size() == 1 && pt3p.size() == 2)
		{
			Triangle3 t = Triangle3(PointList[FaceList[i][0]], PointList[FaceList[i][1]], PointList[FaceList[i][2]]);
			CGAL::cpp11::result_of<Intersect_3(Plane3, Triangle3)>::type result = intersection(plane3, t);
			Segment3* s3 = boost::get<Segment3>(&*result);

			PointList.push_back((Point3)(*s3)[0]);
			PointList.push_back((Point3)(*s3)[1]);

			NewPointsList3.push_back((Point3)(*s3)[0]);
			NewPointsList3.push_back((Point3)(*s3)[1]);

			p[PointList.size() * 3 - 6] = ((Point3)(*s3)[0])[0];
			p[PointList.size() * 3 - 5] = ((Point3)(*s3)[0])[1];
			p[PointList.size() * 3 - 4] = ((Point3)(*s3)[0])[2];
			p[PointList.size() * 3 - 3] = ((Point3)(*s3)[1])[0];
			p[PointList.size() * 3 - 2] = ((Point3)(*s3)[1])[1];
			p[PointList.size() * 3 - 1] = ((Point3)(*s3)[1])[2];

			std::vector<Point2> tmpPoints2;
			
			if (fabs(PointList[fcp[0]][0] - PointList[PointList.size() - 1][0])  < ACCURACYLIMIT && fabs(PointList[fcp[0]][0] - PointList[PointList.size() - 2][0])  < ACCURACYLIMIT)
			{
				tmpPoints2.push_back(Point2(PointList[fcp[0]][1], PointList[fcp[0]][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][1], PointList[PointList.size() - 1][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][1], PointList[PointList.size() - 2][2]));
				tmpPoints2.push_back(Point2(PointList[fcp[1]][1], PointList[fcp[1]][2]));
			}
			else if (fabs(PointList[fcp[0]][1] - PointList[PointList.size() - 1][1]) < ACCURACYLIMIT &&fabs(PointList[fcp[0]][1] - PointList[PointList.size() - 2][1]) < ACCURACYLIMIT)
			{
				tmpPoints2.push_back(Point2(PointList[fcp[0]][0], PointList[fcp[0]][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][2]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][2]));
				tmpPoints2.push_back(Point2(PointList[fcp[1]][0], PointList[fcp[1]][2]));
			}
			else
			{
				tmpPoints2.push_back(Point2(PointList[fcp[0]][0], PointList[fcp[0]][1]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][1]));
				tmpPoints2.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][1]));
				tmpPoints2.push_back(Point2(PointList[fcp[1]][0], PointList[fcp[1]][1]));
			}

			Triangulation T;
			T.insert(tmpPoints2.begin(), tmpPoints2.end());
			Triangulation::Finite_faces_iterator Finite_face_iterator;

			for (Finite_face_iterator = T.finite_faces_begin(); Finite_face_iterator != T.finite_faces_end(); ++Finite_face_iterator)
			{
				Point2 p0 = Point2(T.triangle(Finite_face_iterator).vertex(0));
				Point2 p1 = Point2(T.triangle(Finite_face_iterator).vertex(1));
				Point2 p2 = Point2(T.triangle(Finite_face_iterator).vertex(2));

				std::vector<Point2>::iterator iter;
				int F[3]; 
				for (iter = tmpPoints2.begin(); iter != tmpPoints2.end(); iter++)
				{
					if (fabs((*iter)[0] - p0[0])< ACCURACYLIMIT && fabs((*iter)[1] - p0[1])< ACCURACYLIMIT)
					{

						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[0] = fcp[0];
							break;
						case 1:
							F[0] = PointList.size() - 1;
							break;
						case 2:
							F[0] = PointList.size() - 2;
							break;
						case 3:
							F[0] = fcp[1];
							break;

						}
					}
					if (fabs((*iter)[0] - p1[0])< ACCURACYLIMIT && fabs((*iter)[1] - p1[1])< ACCURACYLIMIT)
					{
	
						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[1] = fcp[0];
							break;
						case 1:
							F[1] = PointList.size() - 1;
							break;
						case 2:
							F[1] = PointList.size() - 2;
							break;
						case 3:
							F[1] = fcp[1];
							break;

						}
					}
					if (fabs((*iter)[0] - p2[0])< ACCURACYLIMIT && fabs((*iter)[1] - p2[1])< ACCURACYLIMIT)
					{
		
						switch (std::distance(tmpPoints2.begin(), iter))
						{
						case 0:
							F[2] = fcp[0];
							break;
						case 1:
							F[2] = PointList.size() - 1;
							break;
						case 2:
							F[2] = PointList.size() - 2;
							break;
						case 3:
							F[2] = fcp[1];
							break;

						}
					}
					
				}

				f2[side2] = F[0];
				side2++;
				f2[side2] = F[1];
				side2++;
				f2[side2] = F[2];
				side2++;

			}

			////////////////

			std::vector<Point2> tmpPoints21;
			Triangulation T1;

			if (fabs(PointList[fcn[0]][0] - PointList[PointList.size() - 1][0])< ACCURACYLIMIT&& fabs(PointList[fcn[0]][0] - PointList[PointList.size() - 2][0]) < ACCURACYLIMIT)
			{
				tmpPoints21.push_back(Point2(PointList[fcn[0]][1], PointList[fcn[0]][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][1], PointList[PointList.size() - 1][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][1], PointList[PointList.size() - 2][2]));
			}
			else if (fabs(PointList[fcn[0]][1] - PointList[PointList.size() - 1][1]) < ACCURACYLIMIT &&fabs(PointList[fcn[0]][1] - PointList[PointList.size() - 2][1]) < ACCURACYLIMIT)
			{
				tmpPoints21.push_back(Point2(PointList[fcn[0]][0], PointList[fcn[0]][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][2]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][2]));
			}
			else
			{
				tmpPoints21.push_back(Point2(PointList[fcn[0]][0], PointList[fcn[0]][1]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 1][0], PointList[PointList.size() - 1][1]));
				tmpPoints21.push_back(Point2(PointList[PointList.size() - 2][0], PointList[PointList.size() - 2][1]));
			}

			T1.insert(tmpPoints21.begin(), tmpPoints21.end());
			Triangulation::Finite_faces_iterator Finite_face_iterator1;


			for (Finite_face_iterator1 = T1.finite_faces_begin(); Finite_face_iterator1 != T1.finite_faces_end(); ++Finite_face_iterator1)
			{
				Point2 p0 = Point2(T1.triangle(Finite_face_iterator1).vertex(0));
				Point2 p1 = Point2(T1.triangle(Finite_face_iterator1).vertex(1));
				Point2 p2 = Point2(T1.triangle(Finite_face_iterator1).vertex(2));

				std::vector<Point2>::iterator iter;
				int F[3];
				for (iter = tmpPoints21.begin(); iter != tmpPoints21.end(); iter++)
				{
					if (fabs((*iter)[0] - p0[0])< ACCURACYLIMIT && fabs((*iter)[1] - p0[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[0] = fcn[0];
							break;
						case 1:
							F[0] = PointList.size() - 1;
							break;
						case 2:
							F[0] = PointList.size() - 2;
							break;

						}
					}
					if (fabs((*iter)[0] - p1[0])< ACCURACYLIMIT && fabs((*iter)[1] - p1[1])< ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[1] = fcn[0];
							break;
						case 1:
							F[1] = PointList.size() - 1;
							break;
						case 2:
							F[1] = PointList.size() - 2;
							break;
						}
					}
					if (fabs((*iter)[0] - p2[0]) < ACCURACYLIMIT && fabs((*iter)[1] - p2[1]) < ACCURACYLIMIT)
					{
						switch (std::distance(tmpPoints21.begin(), iter))
						{
						case 0:
							F[2] = fcn[0];
							break;
						case 1:
							F[2] = PointList.size() - 1;
							break;
						case 2:
							F[2] = PointList.size() - 2;
							break;

						}
					}
				}

				f1[side1] = F[0];
				side1++;
				f1[side1] = F[1];
				side1++;
				f1[side1] = F[2];
				side1++;
			}

		}

	}

	std::vector<Point2> NewPointsList2(NewPointsList3.size());

	for (unsigned i = 0; i < NewPointsList3.size(); i++)
	{
		if (NewPointsList3[i] != NULLPOINT3)
		{
			if (fabs(m[0] - m[3]) < ACCURACYLIMIT && fabs(m[0] - m[6]) < ACCURACYLIMIT && fabs(m[3] - m[6]) <ACCURACYLIMIT)
			{
				NewPointsList2.at(i) = Point2(NewPointsList3[i][1], NewPointsList3[i][2]);
			}
			else if (fabs(m[1] - m[4]) < ACCURACYLIMIT && fabs(m[1] - m[7])  < ACCURACYLIMIT && fabs(m[4] - m[7]) < ACCURACYLIMIT)
			{
				NewPointsList2.at(i) = Point2(NewPointsList3[i][0], NewPointsList3[i][2]);
			}
			else
			{
				NewPointsList2.at(i) = Point2(NewPointsList3[i][0], NewPointsList3[i][1]);
			}
		}
	
	}
	std::vector<Point2> tmpPoint2,nullp(1);
	std::vector<Point2>::iterator pit;
	for (pit = NewPointsList2.begin(); pit != NewPointsList2.end(); pit++)
	{
		if (*pit != nullp[0])
		{
			tmpPoint2.push_back(*pit);
		}
	}

	Triangulation TCenter;
	TCenter.insert(tmpPoint2.begin(), tmpPoint2.end());
	Triangulation::Finite_faces_iterator Finite_face_iterator;
	for (Finite_face_iterator = TCenter.finite_faces_begin(); Finite_face_iterator != TCenter.finite_faces_end(); ++Finite_face_iterator)
	{
		Point2 p0 = Point2(TCenter.triangle(Finite_face_iterator).vertex(0));
		Point2 p1 = Point2(TCenter.triangle(Finite_face_iterator).vertex(1));
		Point2 p2 = Point2(TCenter.triangle(Finite_face_iterator).vertex(2));
		
		std::vector<Point2>::iterator iter;
		int F[3];
		int ii = 0;
		for (iter = NewPointsList2.begin(); iter != NewPointsList2.end(); iter++)
		{
			if (*iter!= nullp[0])
			{
				if (fabs((*iter)[0] - p0[0])< ACCURACYLIMIT && fabs((*iter)[1] - p0[1])< ACCURACYLIMIT)
				{
					ii++;
					F[0] = std::distance(NewPointsList2.begin(), iter);
				}
				if (fabs((*iter)[0] - p1[0])< ACCURACYLIMIT && fabs((*iter)[1] - p1[1])< ACCURACYLIMIT)
				{
					ii++;
					F[1] = std::distance(NewPointsList2.begin(), iter);
				}
				if (fabs((*iter)[0] - p2[0])< ACCURACYLIMIT && fabs((*iter)[1] - p2[1])< ACCURACYLIMIT)
				{
					ii++;
					F[2] = std::distance(NewPointsList2.begin(), iter);
				}
			}		
		
		}
		
		if (ii == 2)
		{
			return p1[1];
		}

		f1[side1] = F[0];
		side1++;
		f1[side1] = F[1];
		side1++;
		f1[side1] = F[2];
		side1++;

		f2[side2] = F[0];
		side2++;
		f2[side2] = F[1];
		side2++;
		f2[side2] = F[2];
		side2++;

	}


	sides[0] = side1 / 3;
	sides[1] = side2 / 3;
	outpn[0] = PointList.size();

	return 0;

}
