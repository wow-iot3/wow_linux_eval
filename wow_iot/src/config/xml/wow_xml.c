#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file/wow_file.h"
#include "file/wow_hfile.h"

#include "config/wow_xml.h"

#include "ezxml.h"

/* brief    解析xml文件
 * param :  pcFile:文件名称完整路径
 * return:  成功返回xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT  wow_xml_init_by_file(char *pcFile)
{
	CHECK_RET_VAL_ERRNO_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return (Xml_PT)ezxml_parse_file(pcFile);
}

/* brief    解析xml格式字符串
 * param :  file:文件名称完整路径
 * return:  成功返回xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT  wow_xml_init_by_str(char *pStr)
{
	CHECK_RET_VAL_ERRNO_P(pStr && strlen(pStr) > 0,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return (Xml_PT)ezxml_parse_str(pStr, strlen(pStr));
}

/* brief    删除xml节点内容与其子内容
 * param :  ptXml:xml操作符
 * return:  无
 */
__EX_API__ void wow_xml_exit(Xml_PT* ptXml)
{
	CHECK_RET_VOID(ptXml && *ptXml);
	ezxml_t xml = (ezxml_t)*ptXml;
	
	ezxml_free(xml);
	*ptXml = NULL;
}



/* brief    创建xml子节点
 * param :  ptItem:子节点名称
 * return:  成功返回xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT wow_xml_new_item(const char *ptItem)
{
	CHECK_RET_VAL_ERRNO_P(ptItem,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	return ezxml_new(ptItem);
}

/* brief    设置xml节点内容
 * param :  ptXml:xml操作符
 * param :  pcText:节点内容
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_xml_set_txt(Xml_PT ptXml,char *pcText)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_P(xml && pcText,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(xml && pcText,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return ezxml_set_txt(xml,pcText)?0:-1;
}

/* brief    设置xml节点属性信息
 * param :  ptXml:xml操作符
 * param :  pcKey:属性节点名称
 * param :  pcVal:属性节点值
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_xml_set_attr(Xml_PT ptXml, const char *pcKey,const char *pcVal)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pcVal,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return ezxml_set_attr(xml,pcKey,pcVal)? 0:-1;

}
static size_t xml_itme_size(ezxml_t xml)
{
	size_t idx = 0;
	ezxml_t node = xml->child;
	for (; node && idx; idx++) node = node->sibling;

	return idx;
}

/* brief    添加xml节点
 * param :  ptXml:父节点xml操作符
 * param :  ptItem:  待添加xml操作符
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_xml_add_item_entry(Xml_PT ptXml,Xml_PT ptItem)
{
	ezxml_t xml   = (ezxml_t)ptXml;
	ezxml_t item  = (ezxml_t)ptItem;
	CHECK_RET_VAL_P(xml && item,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return ezxml_insert(item,xml,xml_itme_size(xml)) ? 0:-1;
}

/* brief    添加xml子节点
 * param :  ptXml:父节点xml操作符
 * param :  pElement: 子节点元素名称
 * param :  pcText:节点文本内容
 * param :  pcKey:属性节点名称
 * param :  pValue:属性节点值
 * return:  成功返回xml操作符 失败返回NULL
 */				  
__EX_API__ Xml_PT wow_xml_add_item_info(Xml_PT ptXml,const char *pElement,
					 const char* pcText,const char *pcKey,const char *pcVal)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pElement,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	ezxml_t node = ezxml_add_child(xml,pElement,xml_itme_size(xml));
	CHECK_RET_VAL_P(node,NULL,"ezxml_add_child failed!\n");

	if(pcText){
		ezxml_t ret = ezxml_set_txt(node,pcText);
		CHECK_RET_GOTO_P(ret,out,"ezxml_set_txt failed!\n");
	}

	if(pcKey && pcVal){
		ezxml_t ret = ezxml_set_attr(node,pcKey,pcVal);
	    CHECK_RET_GOTO_P(ret,out,"ezxml_set_attr failed!\n");
	}
	return node;
out:
	ezxml_free(node);
	return NULL;
}


/* brief    设置xml节点内容
 * param :  ptXml:xml操作符
 * return:  成功返回节点内容 失败返回NULL
 */
__EX_API__ const char* wow_xml_get_txt(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return (const char*)xml->txt;
}

/* brief    设置xml节点名称
 * param :  ptXml:xml操作符
 * return:  成功返回节点名称 失败返回NULL
 */
__EX_API__ const char* wow_xml_get_name(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return (const char*)xml->name;
}

/* brief    获取xml节点值
 * param :  ptXml:xml操作符
 * param :  pcKey:属性节点名称
 * return:  成功返回属性节点值 失败返回NULL
 */
__EX_API__ const char* wow_xml_get_attr(Xml_PT ptXml, const char *pcKey)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pcKey,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return ezxml_attr(xml,pcKey);
}

/* brief    获取xml的第一个子节点
 * param :  ptXml:父节点xml操作符
 * return:  成功返回子节点xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT wow_xml_firstChild(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return (Xml_PT)xml->child;
}

/* brief    获取xml的下一个节点
 * param :  ptXml:节点xml操作符
 * return:  成功返回节点xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT wow_xml_nextSibling(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	return (Xml_PT)ezxml_next(xml);
}


/* brief    通过name获取xml的子节点
 * param :  ptXml:节点xml操作符
 * param :  name:节点xml名称
 * return:  成功返回节点xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT wow_xml_findName(Xml_PT ptXml, const char *pcName)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return (Xml_PT)ezxml_child(xml,pcName);
}

/* brief    通过路径获取xml的子节点
 * param :  ptXml:节点xml操作符
 * param ： pVarfmt:占位符 s对应字符串和i整数
 * return:  成功返回节点xml操作符 失败返回NULL
 */
__EX_API__ Xml_PT wow_xml_findPath(Xml_PT ptXml,const char *pVarfmt, ...)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pVarfmt,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	va_list vl;
	va_start(vl, pVarfmt);
	const char *type = pVarfmt;
	while (*type) {
		switch (*type++) {
		case 's': xml = ezxml_child(xml, va_arg(vl, char*)); break;
		case 'i': xml = ezxml_idx(xml->child, va_arg(vl, int)); break;
		default:  xml = NULL;break;
		}
		CHECK_RET_BREAK(xml);
	}
	va_end(vl);
	
	return xml;
}

/* brief    移除xml字符
 * param :  ptXml :需移除的xml操作符
 * return:  无
 */
__EX_API__ void wow_xml_remove(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VOID(xml);

    ezxml_remove(xml);
}

/* brief    获取xml字符
 * param :  ptXml :需保存的xml操作符
 * return:  成功返回字符内容 失败返回NULL
 */
__EX_API__ char* wow_xml_print(Xml_PT ptXml)
{
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_ERRNO_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    return ezxml_toxml(xml);
}

/* brief    将其保存为xml文件
 * param :  ptXml :需保存的xml操作符
 * param :  pcFile:文件名称完整路径
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_xml_save(Xml_PT ptXml,char *pcFile)
{
	int ret = 0;
	ezxml_t xml = (ezxml_t)ptXml;
	CHECK_RET_VAL_P(xml,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    char* str = ezxml_toxml(xml);
	CHECK_RET_VAL_P(str, -1,"ezxml_toxml failed!\n");
	
	File_T* file = wow_hfile_open(FILE_FIO,pcFile, FILE_CREATE|FILE_RDWR);
	CHECK_RET_VAL_P(file, -FILE_OPEN_FAILED,"wow_file_open %s failed\n",pcFile);
	ret = wow_hfile_write(file,str,strlen(str));
	wow_hfile_close(file);

	ret = (ret == strlen(str))?0:-FILE_WRITE_FAILED;
    free(str);
	
	return ret;
}

