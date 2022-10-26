#include "lltc-resilient-routes-generation.hpp"

using namespace lltc;

ResilientRouteGenerationGeneric::ResilientRouteGenerationGeneric() {

}

ResilientRouteGenerationGeneric::~ResilientRouteGenerationGeneric() {

}

LltcResilientRouteVectors* ResilientRouteGenerationGeneric::genResilientRoutesVectors
									(LltcGraph* g, int srcNodeId, LltcConfiguration* config) {
	return NULL;
}

ResilientRoutes* ResilientRouteGenerationGeneric::constructResilientRoutes(LltcGraph* g,
											LltcResilientRouteVectors* rrv, int dstNodeId,
																LltcConfiguration* config) {
	return NULL;
}
