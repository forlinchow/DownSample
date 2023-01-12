#pragma once
#include <vector>
#include <list>
#include <algorithm>
#include <string>

#include "definitions.h"
namespace forlin {

	//��ʽ�ļ�ͷ����¼����ʱ��Ͱ汾�ŵ�����
	struct _Header
	{
		std::string Version;				//��ʽ�汾��
		std::string CreateTime;				//����ʱ��
		std::string LastestTime;			//�����޸�ʱ��
		std::string AttributeTable;			//���Ա�
		size_t numOfPoints;					//������
		float minx, miny, minz, maxx, maxy, maxz;

	};

	struct _XYZRGB
	{
		float x, y, z;
		unsigned char r, g, b;
		POINTTYPE type() {
			return POINTTYPE::XYZRGB;
		}
	};

	struct _XYZ
	{
		float x, y, z;
		POINTTYPE type() {
			return POINTTYPE::XYZ;
		}
	};

	struct _XYZRGBI
	{
		float x, y, z;
		unsigned char r, g, b,i;
		POINTTYPE type() {
			return POINTTYPE::XYZRGBI;
		}
	};

	template <typename PointT>
	struct subData
	{
		std::list<PointT> data;
	};

	template <typename PointT>
	struct BlockData {
		std::vector<subData<PointT>> _allData;
		std::vector<std::string> files;
	};

	//��vector��ĵ���ת��ΪBlock����
	template<typename PointT>
	void TransferPointsToBlock(std::vector<PointT>* datas, forlin::BlockData<PointT>* data, std::string outFolder) {
		std::string outdir;
		forlin::Converter<PointT> tC;
		tC.setOutFolder(outFolder);

		int _iterator;
		size_t _end = datas->size();
		for (size_t i = 0; i < _end; i++)
		{
			outdir = tC.PointToIndexOfFYData((*datas)[i].x, (*datas)[i].y, (*datas)[i].z);
			auto index = std::find(data->files.begin(), data->files.end(), outdir);
			if (index != data->files.end())
			{
				//found
				_iterator = std::distance(data->files.begin(), index);
				//data._allData[i].data.push_back(k);
				data->_allData[_iterator].data.push_back((*datas)[i]);
			}
			else {
				//not found
				data->files.push_back(outdir);
				forlin::subData<PointT> newsub;
				data->_allData.push_back(newsub);
				data->_allData[data->_allData.size() - 1].data.push_back((*datas)[i]);
			}
		}
		//std::cout << "processing done!!!!!!!!!!!!" << std::endl;
		std::cout << "buffer data process done!" << std::endl;
	}

	//��block����д�뵽FYData��
	template <typename PointT>
	void writeBlockToFYData(BlockData<PointT> * data)
	{
		std::cout << "����д����" << std::endl;
		for (int j = 0; j < data->files.size(); j++) {
			FILE *out = fopen(data->files[j].c_str(), "ab+");
			if (out == NULL)
			{
				std::cout << "open "<< data->files[j]<<" error" << std::endl;
				continue;
			}

			for (auto k = data->_allData[j].data.begin(); k != data->_allData[j].data.end(); k++)
			{
				fwrite(&(*k), sizeof(PointT), 1, out);
			}
			fclose(out);
		}
	}

	template<typename PointT>
	class PointReader
	{
	public:
		virtual ~PointReader() {};

		//�жϿɶ�ȡ�����ݿ��С
		virtual int BlockReady() = 0;

		virtual int LeftBlock() = 0;

		virtual int MovedBlocks() = 0;

		virtual int readfile(std::string path, std::string savepath)=0;
		//��ȡһ�����ݿ�
		
		virtual std::vector<PointT> getOneBlock() = 0;

	protected:
		

	};
}