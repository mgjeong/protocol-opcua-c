
/**
 * @file
 *
 * This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_UTILS_H_
#define EDGE_UTILS_H_

typedef void *keyValue;

typedef struct edgeMapNode
{
  /** Map Key.*/
  keyValue key;

  /** map key-value pair.*/
  keyValue value;

  /** Next node in list.*/
  struct edgeMapNode *next;
} edgeMapNode;

typedef struct edgeMap
{
   /** Map Head.*/
  edgeMapNode* head;
} edgeMap;


/**
 * Insert key-value pair into the edge util map
 *
 * @return edgeMap
 */
edgeMap* createMap();

/**
 * Insert key-value pair into the edge util map
 *
 * @param map
 * @param key
  * @param value
 * @return
 */
void insertMapElement(edgeMap *map, keyValue key, keyValue value);

/**
 * Get element value from the edge util map
 *
 * @param map 
 * @param key 
 * @return keyValue
 */
keyValue getMapElement(edgeMap *map, keyValue key);

/**
 * Delete and free memory of the edge util map
 *
 * @param map 
 * @return keyValue
 */
void deleteMap(edgeMap *map);

#endif /* EDGE_UTILS_H_ */
