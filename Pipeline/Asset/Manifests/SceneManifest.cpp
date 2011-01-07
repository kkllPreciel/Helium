#include "SceneManifest.h"

using namespace Helium;
using namespace Helium::Asset;

REFLECT_DEFINE_CLASS(SceneManifest);

void SceneManifest::AcceptCompositeVisitor( Reflect::Composite& comp )
{
    comp.AddField( &SceneManifest::m_Assets, TXT( "m_Assets" ) );
}