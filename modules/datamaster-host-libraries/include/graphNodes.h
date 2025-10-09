#pragma once

#include <carpeDM.h>

/**
 * Merge sub-IDs into a single ID string for a timing message vertex
 * @param vertex The vertex containing sub-ID fields
 * @return A string representing the merged ID in hexadecimal format
 */
std::string MergeSubIDsIntoID( const myVertex& vertex );

/**
 * Split a single ID string into sub-ID fields for a timing message vertex
 * @param id The ID string to split
 * @param vertex The vertex to populate with sub-ID fields
 */
void SplitIDIntoSubIDs( const std::string& id, myVertex& vertex );

void SyncIdSubIds( myVertex& vertex );

/**
 * Populate the np (node pointer) field of each vertex in the graph
 * @param g The graph to initialize
 */
void InitializeGraphNodes( Graph& g );
