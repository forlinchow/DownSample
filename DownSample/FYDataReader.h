#pragma once

#include <atomic>
#include <direct.h>
#include <list>
#include "stuff.h"
#include "PointReader.h"
#include "definitions.h"
#include <thread>
using std::atomic;

template <typename PointT>
void DownSample2(float SpaceDistance, boost::shared_ptr<pcl::PointCloud<PointT>> cloud, boost::shared_ptr<pcl::PointCloud<PointT>> cloud_downsample) {
// 	std::cerr << "PointCloud before filtering: " << cloud->points.size()
// 		<< " data points (" << pcl::getFieldsList(*cloud) << ").";
	//����kdtree
	pcl::KdTreeFLANN<PointT> kdtree;
	kdtree.setInputCloud(cloud);

	float radius = SpaceDistance;

	std::vector<int> pointIdxRadiusSearch;
	std::vector<float> pointRadiusSquaredDistance;

	std::vector<int> idxForPointCloud(cloud->points.size());

	for (int i = 0; i < idxForPointCloud.size(); i++)
	{
		if (idxForPointCloud[i] == 0) {
			cloud_downsample->points.push_back(cloud->points[i]);
			if (kdtree.radiusSearch(cloud->points[i], radius, pointIdxRadiusSearch, pointRadiusSquaredDistance) > 0)
			{
				for (size_t j = 0; j < pointIdxRadiusSearch.size(); ++j)
					idxForPointCloud[pointIdxRadiusSearch[j]] = 1;
			}
		}
	}
// 	std::cerr << "PointCloud after filtering: " << cloud_downsample->points.size()
// 		<< " data points (" << pcl::getFieldsList(*cloud_downsample) << ").";
}

//��ϡ�̺߳���
template<typename PointT>
void ThreadDownSample(string filename, float dis, string InputDir, string OutDir, atomic<int>& resourceofCPU) {
	PointT temp;
	if (temp.type() == forlin::POINTTYPE::XYZRGB)
	{
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_downsample(new pcl::PointCloud<pcl::PointXYZRGB>);


		FILE *in = fopen((InputDir.append("\\").append(filename)).c_str(), "rb");
		if (in == NULL)
		{
			std::cout << "open file:" << filename << "error" << std::endl;
			exit(-1);
		}
		pcl::PointXYZRGB tpclPoint;
		while (!feof(in))
		{
			fread(&temp, sizeof(PointT), 1, in);
			tpclPoint.x = temp.x; tpclPoint.y = temp.y; tpclPoint.z = temp.z;
			tpclPoint.r = temp.r; tpclPoint.g = temp.g; tpclPoint.b = temp.b;
			cloud->points.push_back(tpclPoint);
		}
		fclose(in);

		//��ϡ����
		DownSample2<pcl::PointXYZRGB>(dis, cloud, cloud_downsample);

		FILE *out = fopen((OutDir.append("\\").append(filename)).c_str(), "wb");
		if (out == NULL)
		{
			std::cout << "open file:" << filename << "error" << std::endl;
			exit(-1);
		}
		for (int i = 0; i < cloud_downsample->points.size(); i++)
		{
			temp.x = cloud_downsample->points[i].x; temp.y = cloud_downsample->points[i].y; temp.z = cloud_downsample->points[i].z;
			temp.r = cloud_downsample->points[i].r; temp.g = cloud_downsample->points[i].g; temp.b = cloud_downsample->points[i].b;
			fwrite(&temp, sizeof(PointT), 1, out);
		}
		fclose(out);
	}
	resourceofCPU++;
}

//��ϡ�̺߳���,֧��FYDATA�½ṹ
template<typename PointT>
void ThreadDownSample1(string filename, float dis, string InputDir, string OutDir, atomic<int>& resourceofCPU) {
	PointT temp;
	if (temp.type() == forlin::POINTTYPE::XYZRGB)
	{
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_downsample(new pcl::PointCloud<pcl::PointXYZRGB>);


		FILE *in = fopen(filename.c_str(), "rb");
		if (in == NULL)
		{
			std::cout << "open file:" << filename << "error" << std::endl;
			exit(-1);
		}
		pcl::PointXYZRGB tpclPoint;
		while (!feof(in))
		{
			fread(&temp, sizeof(PointT), 1, in);
			tpclPoint.x = temp.x; tpclPoint.y = temp.y; tpclPoint.z = temp.z;
			tpclPoint.r = temp.r; tpclPoint.g = temp.g; tpclPoint.b = temp.b;
			cloud->points.push_back(tpclPoint);
		}
		fclose(in);

		//��ϡ����
		DownSample2<pcl::PointXYZRGB>(dis, cloud, cloud_downsample);

		FILE *out = fopen((OutDir.append(filename.substr(InputDir.length()))).c_str(), "wb");
		if (out == NULL)
		{
			std::cout << "open file:" << filename << "error" << std::endl;
			exit(-1);
		}
		for (int i = 0; i < cloud_downsample->points.size(); i++)
		{
			temp.x = cloud_downsample->points[i].x; temp.y = cloud_downsample->points[i].y; temp.z = cloud_downsample->points[i].z;
			temp.r = cloud_downsample->points[i].r; temp.g = cloud_downsample->points[i].g; temp.b = cloud_downsample->points[i].b;
			fwrite(&temp, sizeof(PointT), 1, out);
		}
		fclose(out);
	}
	resourceofCPU++;
}

namespace forlin {

	// 	struct FYFile
	// 	{
	// 		string filename;
	// 		float size;
	// 	};


	template<typename PointT>
	class FYDataReader
	{
	public:
		FYDataReader() {
			FYDataDir = "";
			OutputDir = "";
			//������Դ����
			numOfCPUResources = forlin::_getNumOfProcessors() <= 2 ? 1 : forlin::_getNumOfProcessors();
		};

		FYDataReader(string DataDir, string OutDir, float DownDis) {
			FYDataDir = DataDir;
			OutputDir = OutDir;
			downSampleDis = DownDis;
			numOfCPUResources = forlin::_getNumOfProcessors() <= 2 ? 1 : forlin::_getNumOfProcessors();
		}

		~FYDataReader() {

		};

		//���̳߳�ϡ����
		void MultiThreadRead() {
			if (FYDataDir == "" || OutputDir == "")
			{
				message("Please input the right FYDataDir");
				exit(-1);
			}
			std::vector<string> fileList;
			getFiles(FYDataDir, fileList);
			while (fileList.size() > 0)
			{
				string t, tO;
				string FYfile = fileList.back();
				if (_getFileSize(t.assign(FYDataDir).append("\\").append(FYfile)).QuadPart < 2048)
				{
					copy(t.assign(FYDataDir).append("\\").append(FYfile).c_str(), tO.assign(OutputDir).append("\\").append(FYfile).c_str());
				}
				else
				{
					while (numOfCPUResources == 0)
					{
						Sleep(100);
					}
					numOfCPUResources--;
 					std::thread tThread = std::thread(ThreadDownSample<_XYZRGB>, FYfile, this->downSampleDis, \
						this->FYDataDir, this->OutputDir, std::ref(this->numOfCPUResources));
					tThread.detach();
				}
				fileList.pop_back();
			}

			while (numOfCPUResources!=( forlin::_getNumOfProcessors() <= 2 ? 1 : forlin::_getNumOfProcessors()))
			{
				std::cout << "waitting last worker" <<numOfCPUResources<< std::endl;
				Sleep(500);
			}
		};

		//���̳߳�ϡ����,֧����FYData�ṹ
		void MultiThreadRead1() {
			if (FYDataDir == "" || OutputDir == "")
			{
				message("Please input the right FYDataDir");
				exit(-1);
			}
			std::vector<string> fileList;
			getFilesAbsolutePath(FYDataDir, fileList);
			
			while (fileList.size() > 0)
			{
				string t, tO;
				string FYfile = fileList.back();

				string targetPath = tO.assign(OutputDir).append(FYfile.substr(FYDataDir.length()));
				char dir[200],drive[10];
				_splitpath(targetPath.c_str(),drive, dir, NULL, NULL);
				string folderT;
				if (!CheckFloder(folderT.assign(drive).append(dir)))exit(-3);

				if (_getFileSize(t.assign(FYfile)).QuadPart < 2048)
				{
					//̫С���ļ�ֱ�Ӹ���ת��
					copy(t.assign(FYfile).c_str(),targetPath.c_str());
				}
				else
				{
					while (numOfCPUResources == 0)
					{
						Sleep(100);
					}
					numOfCPUResources--;
					std::thread tThread = std::thread(ThreadDownSample1<_XYZRGB>, FYfile, this->downSampleDis, \
						this->FYDataDir, this->OutputDir, std::ref(this->numOfCPUResources));
					tThread.detach();
				}
				fileList.pop_back();
			}

			while (numOfCPUResources != (forlin::_getNumOfProcessors() <= 2 ? 1 : forlin::_getNumOfProcessors()))
			{
				std::cout << "waitting last worker" << numOfCPUResources << std::endl;
				Sleep(500);
			}
		};

		//����ļ�·���Ƿ���ڣ��������򴴽�
		bool CheckFloder(string targetPath) {
			if (_access(targetPath.c_str(), 0) != 0) {
				std::cout << "warning: the folder doesn't exist. " << std::endl;
				std::cout << "creating the folder" << std::endl;
				if (_mkdir(targetPath.c_str()) == -1) {
					std::cout << "Create folder fail. Please check the user right." << std::endl;
					exit(-1);
					return false;
				}
				else {
					std::cout << "create success." << std::endl;
				}
				return true;
			}
			else {
				return true;
			}
		}

		//�����ļ�
		void copy(const char* src, const char* dst)
		{
			using namespace std;
			ifstream in(src, ios::binary);
			ofstream out(dst, ios::binary);
			if (!in.is_open()) {
				cout << "error open file " << src << endl;
				exit(EXIT_FAILURE);
			}
			if (!out.is_open()) {
				cout << "error open file " << dst << endl;
				exit(EXIT_FAILURE);
			}
			if (src == dst) {
				cout << "the src file can't be same with dst file" << endl;
				exit(EXIT_FAILURE);
			}
			char buf[2048];
			long long totalBytes = 0;
			while (in)
			{
				//read��in���ж�ȡ2048�ֽڣ�����buf�����У�ͬʱ�ļ�ָ������ƶ�2048�ֽ�
				//������2048�ֽ������ļ���β������ʵ����ȡ�ֽڶ�ȡ��
				in.read(buf, 2048);
				//gcount()������ȡ��ȡ���ֽ�����write��buf�е�����д��out����
				out.write(buf, in.gcount());
				totalBytes += in.gcount();
			}
			in.close();
			out.close();
		}

		//��ȡ�ļ����������ļ���������·����
		void getFiles(string path, vector<string>& files)
		{
			//�ļ����  
			intptr_t  hFile = 0;
			//�ļ���Ϣ  
			struct _finddata_t fileinfo;
			string p;
			if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
			{
				do
				{
					//�����Ŀ¼,����֮  
					//�������,�����б�  
					if ((fileinfo.attrib &  _A_SUBDIR))
					{
						if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
							getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
					}
					else
					{
						files.push_back(fileinfo.name);
						//files.push_back(p.assign(path).append("\\").append(fileinfo.name));	//����·��
					}
				} while (_findnext(hFile, &fileinfo) == 0);
				_findclose(hFile);
			}
		}

		//��ȡ�ļ����������ļ���(��·����
		void getFilesAbsolutePath(string path, vector<string>& files)
		{
			//�ļ����  
			intptr_t  hFile = 0;
			//�ļ���Ϣ  
			struct _finddata_t fileinfo;
			string p;
			if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
			{
				do
				{
					//�����Ŀ¼,����֮  
					//�������,�����б�  
					if ((fileinfo.attrib &  _A_SUBDIR))
					{
						if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
							getFilesAbsolutePath(p.assign(path).append("\\").append(fileinfo.name), files);
					}
					else
					{
						//files.push_back(fileinfo.name);
						files.push_back(p.assign(path).append("\\").append(fileinfo.name));	//����·��
					}
				} while (_findnext(hFile, &fileinfo) == 0);
				_findclose(hFile);
			}
		}
	
		void message(string msg) {
			std::cout << msg << std::endl;
		};

	protected:
		//С�ڸ���ֵ�ĵ��Ʋ����г�ϡ
		float ThresholdSize;

		//�����ļ��б������ļ������ļ���С
		//std::list<FYFile> FileList;
		float downSampleDis;

		std::list<PointT> PointsVec;

		string FYDataDir;

		string OutputDir;

		atomic<int> numOfCPUResources;

	};
}