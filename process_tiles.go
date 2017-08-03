package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

func main() {
	log.SetFlags(0)
	tiles, err := getTiles()
	if err != nil {
		log.Fatal(err)
	}
	for _, tile := range tiles {
		log.Println("processing", tile)
		err := transcodeH265Tile(tile)
		if err != nil {
			log.Println("Error transcoding image", tile, err)
		}
	}
	if err := writeMontageScript(tiles); err != nil {
		log.Fatal(err)
	}
}

func getTiles() (SortString, error) {
	wd, err := os.Getwd()
	if err != nil {
		return nil, err
	}
	dir, err := os.Open(fmt.Sprintf("%s/build/", wd))
	if err != nil {
		return nil, err
	}
	items, err := dir.Readdirnames(0)
	if err != nil {
		return nil, err
	}
	tiles := SortString{}
	for _, file := range items {
		if strings.HasSuffix(file, ".tile") {
			tiles = append(tiles, fmt.Sprintf("%s/build/%s", wd, file))
		}
	}
	return tiles, nil
}

func transcodeH265Tile(tile string) error {
	cmd := exec.Command("ffmpeg", "-i", tile, "-loglevel", "warning", "-frames:v", "1", "-vsync", "vfr", "-q:v", "1", "-an", fmt.Sprintf("%s.jpg", tile))
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin
	err := cmd.Run()
	if err != nil {
		return err
	}
	return nil
}

func writeMontageScript(tiles SortString) error {
	sort.Sort(tiles)
	str := `#!/bin/sh
montage `
	for i := 0; i < len(tiles); i++ {
		str += tiles[i] + ".jpg "
	}
	str += " -geometry 512x512 -border 0 -tile 8x6 final.jpg"
	return ioutil.WriteFile(os.Args[1], []byte(str), os.ModePerm)
}

type SortString []string

func get_int(s string) int {
	sl := strings.Split(filepath.Base(s), ".")
	i, err := strconv.ParseInt(sl[0], 10, 64)
	if err != nil {
		log.Fatal(err)
	}
	return int(i)
}
func (s SortString) Less(i, j int) bool {
	return get_int(s[i]) < get_int(s[j])
}
func (s SortString) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}
func (s SortString) Len() int {
	return len(s)
}
