///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ShaderPipelineDescription
/// Describes a pipeline of tools plus a backend for shader translation. The simple shader
/// generator will convert zilch shaders to spir-v binary, run this binary through the
/// tools, then finally pass it to the backend to get a final shader result.
class ShaderPipelineDescription
{
public:
  typedef Zilch::Ref<ZilchShaderIRTranslationPass> TranslationPassRef;
  typedef Zilch::Ref<ZilchShaderIRBackend> BackendPassRef;

  Array<TranslationPassRef> mToolPasses;
  /// Debug passes run after all of the tool passes. These are not fed
  /// into the backend, but are used to collect any debug information
  /// (such as the validator) on the final spir-v data.
  Array<TranslationPassRef> mDebugPasses;
  BackendPassRef mBackend;

  ZilchRefLink(ShaderPipelineDescription);
};

//-------------------------------------------------------------------SimpleShaderReflectionData
/// Simplified reflection data generated by the simple shader generator. As there can be many
/// passes in a pipeline it can be quite complicated to figure out what where fragment properties
/// actually end up in a final backend's binding points. This structure builds a quick-mapping from
/// shader + fragment + property to final uniform/sampledimage reflection information.
/// This class is meant to be usable for most project to generate final reflection data. If a more
/// complicated shader generation scheme is used then this should also serve as an aid to
/// understand how reflection data is transfered through a pipeline.
class SimplifiedShaderReflectionData
{
public:
  typedef ZilchShaderIRCompositor::ShaderStageDescription ShaderStageDescription;
  typedef ShaderStageInterfaceReflection::SampledImageRemappings SampledImageRemappings;
  typedef Zilch::Ref<ShaderTranslationPassResult> PassResultRef;
  ZilchRefLink(SimplifiedShaderReflectionData);

  /// Describes what uniform buffer a property ends up in and where.
  struct UniformReflectionData
  {
    /// The index into the stage reflection data's uniform buffers.
    size_t mBufferIndex;
    /// The index within the uniform buffer that this member maps to.
    size_t mMemberIndex;
  };

  /// For a sampler, image, or sampled image, defines all resultant samplers, images, and sampled images
  /// that this results in. The indices are used to map into the stage reflection data. Note: this is a
  /// multi-to-multi mapping as any stage can split or merge some of these types together.
  struct SampledImageRemappingData
  {
    Array<int> mSampledImageIds;
    Array<int> mImageIds;
    Array<int> mSamplerIds;
  };

  /// Describes how to find the reflection data for a ssbo.
  struct StructuredStorageBufferRemappingData
  {
    /// The index into the reflection data of the final stage.
    size_t mIndex;
  };

  /// Describes how to find the reflection data for a storage image.
  struct StorageImageRemappingData
  {
    /// The index into the reflection data of the final stage.
    size_t mIndex;
  };

  /// Lookup data for a fragment. Contains information about how to turn a
  /// property into a location in the overall stage reflection data.
  struct FragmentLookup
  {
    /// Map of properties (by name) to locations with a uniform buffer.
    HashMap<String, UniformReflectionData> mMaterialUniforms;
    /// Map of properties (by name) of sampled images. This can turn into any number of samplers, images, and sampled images.
    HashMap<String, SampledImageRemappingData> mSampledImages;
    /// Map of properties (by name) of samplers. This can turn into any number of samplers and images.
    HashMap<String, SampledImageRemappingData> mSamplers;
    /// Map of properties (by name) of images. This can turn into any number of images and sampled images.
    HashMap<String, SampledImageRemappingData> mImages;
    /// Map of properties (by name) of storage images to their reflection data index.
    HashMap<String, StorageImageRemappingData> mStorageImages;
    /// Map of properties (by name) of structured storage buffers to their reflection data index.
    HashMap<String, StructuredStorageBufferRemappingData> mStructedStorageBuffers;
  };

  /// Build the cached reflection data for quick access for a shader.
  /// This will walk all of the results from the passes and 'merge' them together to get a quick jump table. 
  void CreateReflectionData(ZilchShaderIRLibrary* shaderLibrary, ShaderStageDescription& stageDef, Array<PassResultRef>& passResults);

  /// Find the reflection data for a uniform given the fragment and property. Returns null if the property can't be found.
  ShaderResourceReflectionData* FindUniformReflectionData(ZilchShaderIRType* fragmentType, StringParam propertyName);
  /// Finds all potential images, samplers, and sampled images that the given SampledImage property results in.
  void FindSampledImageReflectionData(ZilchShaderIRType* fragmentType, StringParam propertyName, Array<ShaderResourceReflectionData*>& results);
  /// Finds all potential images and sampled images that the given Image property results in.
  void FindImageReflectionData(ZilchShaderIRType* fragmentType, StringParam propertyName, Array<ShaderResourceReflectionData*>& results);
  /// Finds all potential samplers and sampled images that the given Sampler property results in.
  void FindSamplerReflectionData(ZilchShaderIRType* fragmentType, StringParam propertyName, Array<ShaderResourceReflectionData*>& results);
  /// Find the reflection data for a storage image given the fragment and property. Returns null if the property can't be found.
  ShaderResourceReflectionData* FindStorageImage(ZilchShaderIRType* fragmentType, StringParam propertyName);
  /// Find the reflection data for a structed storage buffer given the fragment and property. Returns null if the property can't be found.
  ShaderResourceReflectionData* FindStructedStorageBuffer(ZilchShaderIRType* fragmentType, StringParam propertyName);

  /// Map lookup of fragment name to property information about the fragment.
  HashMap<String, FragmentLookup> mFragmentLookup;
  /// The final reflection data of this shader. Contains all descriptions about interface buffers, uniforms, etc...
  ShaderStageInterfaceReflection mReflection;

private:
  typedef HashMap<String, int> NameToIndexMap;

  // Internal helper class used to remap uniform buffers between stages to each other.
  struct SimpleResourceRemappingData
  {
    size_t mIndex;
    String mName;
    bool mActive;
  };

  // Creates the final mapped data for uniform buffers.
  void CreateUniformReflectionData(ZilchShaderIRLibrary* shaderLibrary, ShaderStageDescription& stageDef, Array<PassResultRef>& passResults);

  // Creates the final mapped data for samplers, images, and sampled images.
  void CreateSamplerAndImageReflectionData(ZilchShaderIRLibrary* shaderLibrary, ShaderStageDescription& stageDef, Array<PassResultRef>& passResults);
  // Given an array of pass results, this recursively walks from the current pass following where
  // all of the input mappings end up at and puts the results in the output mappings.
  void RecursivelyBuildSamplerAndImageMappings(Array<PassResultRef>& passResults, size_t passIndex,
    SampledImageRemappings& inputMappings, SampledImageRemappings& outputMappings);
  // Merges the source image mappings into the destination image mappings.
  void MergeRemappings(SampledImageRemappings& dest, SampledImageRemappings& source);
  /// Convert the final name mappings to actual indices in the reflection data.
  void BuildFinalSampledImageMappings(SampledImageRemappings* resourceMappings, NameToIndexMap& samplerIndices, NameToIndexMap& imageIndices, NameToIndexMap& sampledImageIndices, SampledImageRemappingData& results);

  /// Fills out information for an individual search map (e.g. sampler/image) given the property name.
  void PopulateSamplerAndImageData(HashMap<String, SampledImageRemappingData>& searchMap, StringParam propertyName, Array<ShaderResourceReflectionData*>& results);

  /// Create reflectiond ata for simple opaque types (e.g. storage image and ssbos).
  void CreateSimpleOpaqueTypeReflectionData(ZilchShaderIRLibrary* shaderLibrary, ShaderStageDescription& stageDef, Array<PassResultRef>& passResults);
};

//-------------------------------------------------------------------SimpleZilchShaderGenerator
/// This class is basically a helper wrapper around projects, libraries, the compositor, and the translator.
/// This should give simple examples of how to use the zilch shader translation system.
class SimpleZilchShaderIRGenerator : public Zilch::EventHandler
{
public:
  typedef ZilchSpirVFrontEnd FrontEndTranslatorType;
  typedef Zilch::Ref<ShaderTranslationPassResult> TranslationPassResultRef;
  typedef Zilch::Ref<SimplifiedShaderReflectionData> SimplifiedReflectionRef;

  SimpleZilchShaderIRGenerator(FrontEndTranslatorType* frontEndTranslator);
  SimpleZilchShaderIRGenerator(FrontEndTranslatorType* frontEndTranslator, ZilchShaderSpirVSettings* settings);
  ~SimpleZilchShaderIRGenerator();

  //-------------------------------------------------------------------Settings/Setup
  /// Setup all dependent libraries that we have and compile them into a shader library (only once)
  /// for all subsequent fragment and shader operations. This is not done in the constructor so the 
  /// user has a chance to connect events to this before the dependent libraries are built (in-case any errors happen).
  void SetupDependencies(StringParam extensionsDirectoryPath);
  /// Stores a pipeline description to be used for shader translation.
  void SetPipeline(ShaderPipelineDescription* pipeline);
  /// Clear all projects and reset the translator (clears the library translation)
  void ClearAll();

  //-------------------------------------------------------------------Fragments
  void AddFragmentCode(StringParam fragmentCode, StringParam fileName, void* userData);
  /// Compiles fragments and translates them to an internal representation (spir-v)
  bool CompileAndTranslateFragments();
  void ClearFragmentsProjectAndLibrary();

  //-------------------------------------------------------------------Compositing
  /// Composes the given shader definition of fragments together. Stores the resultant
  /// zilch shader code in the given definition object. The zilch shader code is also cached internally.
  bool ComposeShader(ZilchShaderIRCompositor::ShaderDefinition& shaderDef, ShaderCapabilities& capabilities);

  /// Composes the given shader definition of compute fragments. Stores the resultant
  /// zilch shader code in the given definition object. The zilch shader code is also cached internally.
  /// Compute properties (such as the local workgroup size) should be passed through (can optionally be
  /// null to use the first fragment's properties). 
  bool ComposeComputeShader(ZilchShaderIRCompositor::ShaderDefinition& shaderDef, ShaderCapabilities& capabilities, ZilchShaderIRCompositor::ComputeShaderProperties* computeProperties = nullptr);

  //-------------------------------------------------------------------Shaders
  void AddShaderCode(StringParam shaderCode, StringParam fileName, void* userData);
  /// Compiles shaders and translates them to an internal representation (spir-v)
  bool CompileAndTranslateShaders();
  void ClearShadersProjectAndLibrary();

  //-------------------------------------------------------------------Pipeline compilation.
  // Compiles the shader library through a pipeline of tools to a final backend.

  /// Compiles the current shader library (CompileAndTranslateShaders must be called first)
  /// using the pre-set shader pipeline.
  bool CompilePipeline();
  // Compiles the current shader library (CompileAndTranslateShaders must be called first) given a pipeline.
  bool CompilePipeline(ShaderPipelineDescription& pipeline);
  
  //-------------------------------------------------------------------Reflection Interface
  /// Find a the shader type for a fragment by name. Only checks the fragment library.
  ZilchShaderIRType* FindFragmentType(StringParam typeName);
  /// Find a the shader type for a shader by name. Only checks the shader library.
  ZilchShaderIRType* FindShaderType(StringParam typeName);
  /// Finds the translation result (byte data + reflection data) for a shader.
  ShaderTranslationPassResult* FindTranslationResult(ZilchShaderIRType* shaderType);
  /// Finds the cached simplified reflection data for a shader.
  SimplifiedShaderReflectionData* FindSimplifiedReflectionResult(ZilchShaderIRType* shaderType);

  //-------------------------------------------------------------------Settings
  /// Load default name settings
  static void LoadNameSettings(SpirVNameSettings& nameSettings);
  /// Create the settings that the unit tests use.
  static ZilchShaderSpirVSettings* CreateUnitTestSettings(SpirVNameSettings& nameSettings);
  /// Create the settings that zero uses.
  static ZilchShaderSpirVSettings* CreateZeroSettings(SpirVNameSettings& nameSettings);

  //-------------------------------------------------------------------Internal
  void Initialize(FrontEndTranslatorType* frontEndTranslator, ZilchShaderSpirVSettings* settings);
  void SetupEventConnections();
  // Helper to setup a lot of shared data on the translator
  void SetupTranslator(FrontEndTranslatorType* translator);

  /// Compiles a shader stage with a given pipeline. Caches the results internally.
  /// Also builds the simplified reflection data for this shader.
  bool CompilePipeline(ZilchShaderIRCompositor::ShaderStageDescription& stageDef, ShaderPipelineDescription& pipeline);
  /// Runs the pipeline on one shader type and fills out all of the results for each translation pass.
  /// Also fills out an array of debug results for all of the debug passes in the pipeline.
  /// A debug pass will run on the last pass (before the backend) and typically is used for the validator or disassembler.
  bool CompilePipeline(ZilchShaderIRType* shaderType, ShaderPipelineDescription& pipeline,
    Array<TranslationPassResultRef>& pipelineResults, Array<TranslationPassResultRef>& debugResults);

  void RecursivelyLoadDirectory(StringParam path, ZilchShaderIRProject& project);

  // Basic handler to forward the errors from the underlying
  // project/translator to whoever is listening on this class.
  void OnForwardEvent(Zilch::EventData* e);

  // The shared settings to be used through all of the compositing/translation
  ZilchShaderSpirVSettingsRef mSettings;
  // The front-end translator used.
  FrontEndTranslatorType* mFrontEndTranslator;

  // Core libraries built once.
  ZilchShaderIRLibraryRef mCoreLibrary;
  ZilchShaderIRLibraryRef mShaderIntrinsicsLibrary;
  ZilchShaderIRLibraryRef mExtensionsLibraryRef;

  // The individual fragments to be assembled into shaders
  ZilchShaderIRProject mFragmentProject;
  ZilchShaderIRLibraryRef mFragmentLibraryRef;

  // The shaders built on-top of fragments
  ZilchShaderIRProject mShaderProject;
  ZilchShaderIRLibraryRef mShaderLibraryRef;

  typedef HashMap<String, ZilchShaderIRCompositor::ShaderDefinition> ShaderDefinitionMap;
  /// Keep a mapping of shader name to definition so that we can lookup
  /// the actual shaders for each shader stage (e.g. vertex/pixel).
  ShaderDefinitionMap mShaderDefinitionMap;

  /// Cached translation result for a pipeline run.
  struct ShaderTranslationResult
  {
    /// The final results from running a pipeline.
    TranslationPassResultRef mFinalPassData;
    /// The simplified reflection data.
    SimplifiedReflectionRef mReflectionData;

    /// Maps 1-1 in-order of the debug passes on the pipeline.
    /// Mostly used to collect disassembly and validator results.
    Array<TranslationPassResultRef> mDebugResults;
  };

  /// Map of shaders by name (shader type name) to their translation results.
  HashMap<String, ShaderTranslationResult> mShaderResults;
  /// A pipeline description set by the user to be run on all shader translations.
  Zilch::Ref<ShaderPipelineDescription> mPipeline;
};

}//namespace Zero
