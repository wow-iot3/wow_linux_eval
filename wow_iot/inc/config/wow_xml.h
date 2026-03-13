#ifndef __WOW_IOT_CONFIG_WOW_XML_H_
#define __WOW_IOT_CONFIG_WOW_XML_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Xml_PT;


/* brief    解析xml文件
 * param :  pcFile:文件名称完整路径
 * return:  成功返回xml操作符 失败返回NULL
 */
Xml_PT  wow_xml_init_by_file(char *pcFile);

/* brief    解析xml格式字符串
 * param :  pcStr:文件名称完整路径
 * return:  成功返回xml操作符 失败返回NULL
 */
Xml_PT  wow_xml_init_by_str(char *pcStr);


/* brief    删除xml节点内容与其子内容
 * param :  ptXml:xml操作符
 * return:  无
 */
void wow_xml_exit(Xml_PT* ptXml);


/************************************xml写入操作*****************************/
/* brief    创建xml节点
 * param :  pcElement:节点名称
 * return:  成功返回xml操作符 失败返回NULL
 */
Xml_PT wow_xml_new_item(const char *pcElement);


/* brief    设置xml节点内容
 * param :  ptItem:xml操作符
 * param :  pcText:节点内容
 * return:  成功返回0 失败返回<0
 */
int wow_xml_set_txt(Xml_PT ptItem,char *pcText);


/* brief    设置xml节点属性信息
 * param :  ptItem:xml操作符
 * param :  pcKey:属性节点名称
 * param :  pcVal:属性节点值
 * return:  成功返回0 失败返回<0
 */
int wow_xml_set_attr(Xml_PT ptItem, const char *pcKey,const char *pcVal);


/* brief    添加xml节点
 * param :  ptXml:父节点xml操作符
 * param :  ptItem:  待添加xml操作符
 * return:  成功返回0 失败返回<0
 */
int wow_xml_add_item_entry(Xml_PT ptXml,Xml_PT ptItem);


/* brief    添加xml子节点
 * param :  ptXml:父节点xml操作符
 * param :  pElement: 子节点元素名称
 * param :  pcText:节点文本内容
 * param :  pcKey:属性节点名称
 * param :  pValue:属性节点值
 * return:  成功返回xml操作符 失败返回NULL
 */
Xml_PT wow_xml_add_item_info(Xml_PT ptXml,const char *pElement,
					 const char* pcText,const char *pcKey,const char *pcVal);


/************************************xml读取操作*****************************/
/* brief    设置xml节点内容
 * param :  ptXml:xml操作符
 * return:  成功返回节点内容 失败返回NULL
 */
const char* wow_xml_get_txt(Xml_PT ptXml);

/* brief    设置xml节点名称
 * param :  ptXml:xml操作符
 * return:  成功返回节点名称 失败返回NULL
 */
const char* wow_xml_get_name(Xml_PT ptXml);

/* brief    获取xml节点值
 * param :  ptXml:xml操作符
 * param :  pcKey:属性节点名称
 * return:  成功返回属性节点值 失败返回NULL
 */
const char* wow_xml_get_attr(Xml_PT ptXml, const char *pcKey);

/* brief    通过name获取xml的子节点
 * param :  ptXml:节点xml操作符
 * param :  name:节点xml名称
 * return:  成功返回节点xml操作符 失败返回NULL
 */
Xml_PT wow_xml_findName(Xml_PT ptXml, const char *pName);

/* brief    通过路径获取xml的子节点
 * param :  ptXml:节点xml操作符
 * param ： pVarfmt:占位符 s对应字符串和i整数
 * return:  成功返回节点xml操作符 失败返回NULL
 */
Xml_PT wow_xml_findPath(Xml_PT ptXml,const char *pVarfmt, ...);

/* brief    获取xml的第一个子节点
 * param :  ptXml:父节点xml操作符
 * return:  成功返回子节点xml操作符 失败返回NULL
 */
Xml_PT wow_xml_firstChild(Xml_PT ptXml);

/* brief    获取xml的下一个节点
 * param :  ptXml:节点xml操作符
 * return:  成功返回节点xml操作符 失败返回NULL
 */
Xml_PT wow_xml_nextSibling(Xml_PT ptXml);

/* brief    移除xml字符
 * param :  ptXml :需移除的xml操作符
 * return:  无
 */
void wow_xml_remove(Xml_PT ptXml);


/* brief    获取xml字符
 * param :  ptXml :需保存的xml操作符
 * return:  成功返回字符内容 失败返回NULL
 */
char* wow_xml_print(Xml_PT ptXml);

/* brief    将其保存为xml文件
 * param :  ptXml :需保存的xml操作符
 * param :  pcFile:文件名称完整路径
 * return:  成功返回0 失败返回<0
 */
int wow_xml_save(Xml_PT ptXml,char *pcFile);

#ifdef __cplusplus
}
#endif

#endif

