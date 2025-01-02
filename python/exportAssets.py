import unreal
import os

# Exports all assets from the engine into this directory.
# Maintaining relative paths within the exported directory
export_directory = "/home/ads/assets/steikemann-ue"


def export_static_meshes_as_fbx(export_directory):
    # Ensure the export directory ends with a slash
    if not export_directory.endswith('/'):
        export_directory += '/'

    # Ensure the base export directory exists
    os.makedirs(export_directory, exist_ok=True)

    # Get the asset registry
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    # Define filters for static meshes and textures
    filters = [
        {"class_names": ["StaticMesh"], "extension": ".fbx"},
        {"class_names": ["SkeletalMesh"], "extension": ".fbx"},
        {"class_names": ["Texture2D"], "extension": ".png"},
    ]

    for asset_filter in filters:
        class_names = asset_filter["class_names"]
        file_extension = asset_filter["extension"]

        ar_filter = unreal.ARFilter(
            class_names=class_names,
            recursive_paths=True,
        )
        
        # Get all static mesh assets
        assets = asset_registry.get_assets(ar_filter)

        # FBX Export options
        fbx_export_options = unreal.FbxExportOption()
        fbx_export_options.ascii = False 

        # Iterate through all static meshes and export as FBX
        for asset_data in assets:
            asset_path = asset_data.package_name
            asset_name = asset_data.asset_name

            # Apply path mappings
            relative_path = str(asset_path)
            if relative_path.startswith('/'):
                relative_path = relative_path[1:]
            relative_dir = os.path.dirname(relative_path)  # Extract the relative directory

            # Create the corresponding directory structure in the export location
            full_export_path = os.path.join(export_directory, relative_dir)
            os.makedirs(full_export_path, exist_ok=True)
            unreal.log(f"{full_export_path}")

            export_file = os.path.join(full_export_path, f"{asset_name}{file_extension}") 

            try:
                # Load the static mesh asset
                asset = unreal.load_asset(asset_path)

                # Common export settings
                exporttask = unreal.AssetExportTask()
                exporttask.object = asset
                exporttask.filename = export_file
                exporttask.selected = True
                exporttask.prompt = False
                exporttask.automated = True

                # Set option based on asset type
                if isinstance(asset, unreal.StaticMesh):
                    exporttask.options = fbx_export_options
                elif isinstance(asset, unreal.Texture2D):
                    exporttask.options = unreal.TextureExporterPNG()
                elif isinstance(asset, unreal.SkeletalMesh):
                    exporttask.options = fbx_export_options
                else:
                    unreal.log_warning(f"Skipping {asset_name}, unsupported asset type.")

                if unreal.Exporter.run_asset_export_task(exporttask):
                    unreal.log(f"Successfully exported Texture {asset_name} to {export_file}")
                else:
                    unreal.log_warning(f"Failed to export Texture {asset_name}")


            except Exception as e:
                unreal.log_error(f"Error exporting {asset_path}: {e}")

export_static_meshes_as_fbx(export_directory)
