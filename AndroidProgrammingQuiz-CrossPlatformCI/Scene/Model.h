/**
 * @file    Model.h
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   Abstract base class defining the interface contract for all renderable scene elements.
 *          Every concrete shape (e.g., Triangle, Square) must inherit from this class
 *          and implement its core lifecycle pipeline.
 */

#pragma once

class Model {
    public:
        /**
         * @brief Default constructor for the base Model interface.
         */
        Model()= default;

        /**
         * @brief Virtual destructor ensuring safe, clean derived-class deallocations from memory.
         */
        virtual ~Model() = default;

        /**
         * @brief Pure virtual method for configuring shader pipelines, generating VAO/VBOs,
         *        and uploading initial geometry vertex data to the GPU.
         *        Must be overridden by concrete classes; called exactly once before frame rendering begins.
         */
        virtual  void InitModel() = 0;          // called once, before the first frame

        /**
         * @brief Pure virtual method for executing active OpenGL draw calls.
         *        Must be overridden by concrete classes; called exactly once per display frame.
         */
        virtual  void Render() = 0;             // called once per frame

        /**
         * @brief Virtual callback method invoked whenever the window surface resolution changes.
         *        Provides an empty default implementation so that shapes independent of
         *        viewport aspect ratios do not need to explicitly override it.
         * @param w The new physical width of the surface viewport.
         * @param h The new physical height of the surface viewport.
         */
        virtual  void Resize(int w, int h) {}   // called whenever the surface or window is resized
};
