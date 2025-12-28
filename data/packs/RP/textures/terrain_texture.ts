import { createFile } from "Regolith-Generators"
import { join, extname, basename } from "jsr:@std/path";
import { walkSync } from "jsr:@std/fs";

const ROOT_DIR = Deno.env.get("ROOT_DIR")!;
const modJsonFilePath = join(ROOT_DIR, "..", "mod.json")
const projectNamespace = JSON.parse(Deno.readTextFileSync(modJsonFilePath)).meta.namespace;

const texturesFolder = join(Deno.cwd(), "RP", "textures", "blocks");

const textureData: Record<string, unknown> = {};

for (const entry of walkSync(texturesFolder, { exts: [".png"], includeFiles: true })) {
    const textureName = basename(entry.path, extname(entry.path));
    const relativePath = entry.path.replaceAll("\\", "/").split("/RP/")[1];
    const noExtensionPath = relativePath.replace(extname(relativePath), "");

    textureData[`${projectNamespace}:${textureName}`] = {
        "textures": noExtensionPath
    }
}

createFile({
    "num_mip_levels": 4,
    "padding": 8,
    "resource_pack_name": "pack.name",
    "texture_name": "atlas.terrain",
    texture_data: textureData,
});