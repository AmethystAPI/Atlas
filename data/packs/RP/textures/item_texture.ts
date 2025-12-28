import { createFile } from "Regolith-Generators"
import { join, extname, basename } from "jsr:@std/path";
import { walkSync } from "jsr:@std/fs";

// Project namespace no longer needs to be set here!
// File can just be closed!

const ROOT_DIR = Deno.env.get("ROOT_DIR")!;
const modJsonFilePath = join(ROOT_DIR, "..", "mod.json")
const projectNamespace = JSON.parse(Deno.readTextFileSync(modJsonFilePath)).meta.namespace;

const itemTexturesFolder = join(Deno.cwd(), "RP", "textures", "items");

const textureData: Record<string, unknown> = {};

for (const entry of walkSync(itemTexturesFolder, { exts: [".png"], includeFiles: true })) {
    const textureName = basename(entry.path, extname(entry.path));
    const relativePath = entry.path.replaceAll("\\", "/").split("/RP/")[1];
    const noExtensionPath = relativePath.replace(extname(relativePath), "");

    textureData[`${projectNamespace}:${textureName}`] = {
        "textures": noExtensionPath
    }
}

createFile({
    resource_pack_name: "Atlas RP",
    texture_name: 'atlas.items',
    texture_data: textureData,
});